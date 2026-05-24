#if HEADLESS
#include <algorithm>
#include <filesystem>
#include "application/setups/server/tournament/tournament_coordinator.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/log.h"
#include "augs/misc/randomization.h"
#include "augs/misc/date_time.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/filesystem/file.h"

namespace {
	uint32_t next_pow2(const uint32_t n) {
		uint32_t p = 1;

		while (p < n) {
			p <<= 1;
		}

		return p;
	}

	float skill_from_match_time(const float effective_secs) {
		return 1.0f / std::max(0.001f, effective_secs);
	}
}

tournament_coordinator::tournament_coordinator(
	tournament_config in_cfg,
	augs::path_type in_config_path,
	augs::path_type in_state_path,
	tournament_coordinator_dependencies in_deps
) :
	cfg(std::move(in_cfg)),
	config_path(std::move(in_config_path)),
	state_path(std::move(in_state_path)),
	deps(std::move(in_deps))
{
	const auto current_hash = cfg.compute_hash();

	if (const auto loaded = tournament_state::load(state_path)) {
		if (loaded->config_hash != current_hash) {
			/*
				Config on disk has changed since this ongoing tournament began.
				Pairings, teams or arenas may all differ from what's in the state,
				so resuming would corrupt the bracket. Archive the old state and
				start fresh; the new tournament adopts the current config.
			*/

			LOG(
				"Tournament: config hash mismatch (state=%x, current=%x). "
				"Archiving stale tournament.ongoing.json and starting fresh.",
				loaded->config_hash, current_hash
			);

			try {
				const auto stamp = augs::date_time().get_readable_for_file_long();
				const auto archive_name = std::string("tournament.config-changed.") + stamp + ".json";
				const auto archive_path = state_path.parent_path() / archive_name;

				std::filesystem::rename(state_path, archive_path);
				LOG("Tournament: archived stale state to %x.", archive_path.string());
			}
			catch (const std::exception& err) {
				LOG("Tournament: failed to archive stale state file (%x); removing instead.", err.what());

				try {
					std::filesystem::remove(state_path);
				}
				catch (const std::exception& err2) {
					LOG("Tournament: failed to remove stale state file: %x", err2.what());
				}
			}
		}
		else {
			state = *loaded;
			LOG("Tournament: resumed from %x at stage %x.", state_path.string(), state.stage_index);
		}
	}

	state.config_hash = current_hash;
}

bool tournament_coordinator::finished() const {
	return state.tournament_finished();
}

void tournament_coordinator::initialize_state_if_fresh() {
	if (state.initialized) {
		return;
	}

	const auto N = static_cast<uint32_t>(cfg.teams.size());

	state.teams.clear();
	state.teams.reserve(N);

	std::vector<uint32_t> indices;
	indices.reserve(N);

	for (uint32_t i = 0; i < N; ++i) {
		indices.push_back(i);
	}

	if (cfg.skill_level_source == tournament_skill_source::RANDOM) {
		auto rng = randomization::from_random_device();
		shuffle_range(indices, rng);
	}
	else if (cfg.skill_level_source == tournament_skill_source::ENDPOINT) {
		LOG("Tournament: ENDPOINT skill source not implemented; falling back to ORDER.");
	}

	for (uint32_t rank = 0; rank < N; ++rank) {
		const uint32_t team_idx = indices[rank];

		tournament_team_state ts;
		ts.player_ids = cfg.teams[team_idx];
		ts.skill = static_cast<float>(N - rank);
		ts.eliminated = false;

		state.teams.push_back(std::move(ts));
	}

	state.stage_index = 0;
	state.initialized = true;

	LOG("Tournament: initialized with %x teams.", N);
}

void tournament_coordinator::pair_teams() {
	state.current_stage_matches.clear();

	auto alive = state.surviving_team_indices();

	std::sort(alive.begin(), alive.end(), [this](const uint32_t a, const uint32_t b) {
		return state.teams[a].skill > state.teams[b].skill;
	});

	std::vector<uint32_t> playing = alive;
	std::vector<uint32_t> getting_bye;

	if (state.stage_index == 0) {
		const auto N = static_cast<uint32_t>(alive.size());
		const auto target = next_pow2(N);
		const auto byes = target > N ? target - N : 0u;

		if (byes > 0) {
			if (cfg.byes_to_strongest) {
				getting_bye.assign(alive.begin(), alive.begin() + byes);
				playing.assign(alive.begin() + byes, alive.end());
			}
			else {
				getting_bye.assign(alive.end() - byes, alive.end());
				playing.assign(alive.begin(), alive.end() - byes);
			}

			LOG("Tournament stage 0: %x byes (to %x).", byes, cfg.byes_to_strongest ? "strongest" : "weakest");

			/*
				Apply synthetic match-time skills to bye teams immediately so subsequent
				stages sort them consistently against real match-time winners.

				Only relevant when skill_by_match_time = true; otherwise the source skill
				is authoritative across all stages and bye teams retain their rank naturally.
			*/

			if (cfg.skill_by_match_time) {
				for (uint32_t rank = 0; rank < getting_bye.size(); ++rank) {
					const auto idx = getting_bye[rank];
					const float synthetic_secs =
						cfg.byes_to_strongest ?
						static_cast<float>(rank + 1) :
						static_cast<float>(1e6) * static_cast<float>(rank + 1)
					;

					state.teams[idx].skill = ::skill_from_match_time(synthetic_secs);
				}
			}
		}
	}

	const auto map_index = std::min(static_cast<size_t>(state.stage_index), cfg.arenas.size() - 1);
	const std::string& arena_and_mode_for_stage = cfg.arenas[map_index];

	uint16_t port_cursor = deps.base_port;

	const auto k = playing.size();
	const auto alive_at_stage_start = alive.size();
	const bool byes_this_stage = !getting_bye.empty();

	/*
		Spectators allowed on every match in this stage: tournament players who
		are NOT on any team playing this stage. Includes:
		  - eliminated teams,
		  - bye recipients (stage 0).
		Players assigned to another match this stage are NOT allowed to roam -
		they should be on their own server. assigned_teams kicks any client whose
		id isn't in the roster, so this list does the gatekeeping.
	*/

	std::vector<bool> plays_this_stage(state.teams.size(), false);

	for (const auto idx : playing) {
		plays_this_stage[idx] = true;
	}

	std::vector<std::string> shared_spectator_ids;

	for (uint32_t i = 0; i < state.teams.size(); ++i) {
		if (plays_this_stage[i]) {
			continue;
		}

		for (const auto& pid : state.teams[i].player_ids) {
			shared_spectator_ids.push_back(pid);
		}
	}

	/*
		Players listed in additional_spectators don't take part in the tournament
		but are whitelisted as spectators on every match in every stage - useful
		for casters, observers and admins. They go into every match's roster as
		SPECTATOR; assigned_teams would otherwise kick them as "wrong match".
	*/

	for (const auto& pid : cfg.additional_spectators) {
		shared_spectator_ids.push_back(pid);
	}

	const uint8_t heartbeat_value = [&]() -> uint8_t {
		const auto as_u8 = [](const ranked_server_type t) {
			return static_cast<uint8_t>(t);
		};

		if (alive_at_stage_start == 2) {
			return as_u8(ranked_server_type::TOURNAMENT_FINALS);
		}

		if (alive_at_stage_start == 4) {
			return as_u8(ranked_server_type::TOURNAMENT_SEMIFINALS);
		}

		if (state.stage_index == 0 && byes_this_stage) {
			return as_u8(ranked_server_type::TOURNAMENT_BYES);
		}

		return as_u8(ranked_server_type::TOURNAMENT_MID);
	}();

	for (size_t i = 0; i + 1 < k; i += 2) {
		const uint32_t a =
			cfg.matchup_scheme == tournament_matchup_scheme::STRONGEST_VS_WEAKEST ?
			playing[i / 2] :
			playing[i]
		;

		const uint32_t b =
			cfg.matchup_scheme == tournament_matchup_scheme::STRONGEST_VS_WEAKEST ?
			playing[k - 1 - i / 2] :
			playing[i + 1]
		;

		tournament_match m;
		m.team_a_index = a;
		m.team_b_index = b;
		m.team_a_player_ids = state.teams[a].player_ids;
		m.team_b_player_ids = state.teams[b].player_ids;
		m.spectator_player_ids = shared_spectator_ids;
		m.port = port_cursor++;
		m.arena_and_mode = arena_and_mode_for_stage;
		m.heartbeat_ranked_type = heartbeat_value;
		m.server_name = make_server_name_for(m);

		state.current_stage_matches.push_back(std::move(m));
	}
}

std::string tournament_coordinator::make_server_name_for(const tournament_match& m) const {
	auto stringify = [](const tournament_team_state& t) {
		std::string s = "[";

		for (size_t i = 0; i < t.player_ids.size(); ++i) {
			if (i > 0) {
				s += ", ";
			}

			s += t.player_ids[i];
		}

		s += "]";
		return s;
	};

	return stringify(state.teams[m.team_a_index]) + " vs " + stringify(state.teams[m.team_b_index]);
}

void tournament_coordinator::commit_match_result(
	tournament_match& m,
	const uint32_t winner_team_index,
	const float duration_secs,
	const uint32_t team_a_score,
	const uint32_t team_b_score
) {
	std::scoped_lock lk(state_lk);

	if (m.result.has_value()) {
		return;
	}

	tournament_match_result r;
	r.winner_team_index = winner_team_index;
	r.duration_secs = duration_secs;
	r.team_a_score = team_a_score;
	r.team_b_score = team_b_score;

	m.result = r;

	state.save(state_path);
}

void tournament_coordinator::run_current_stage() {
	if (state.current_stage_matches.empty()) {
		return;
	}

	LOG("Tournament stage %x: running %x matches.", state.stage_index, state.current_stage_matches.size());

	auto commit = [this](
		tournament_match& m,
		const uint32_t winner,
		const float duration,
		const uint32_t team_a_score,
		const uint32_t team_b_score
	) {
		commit_match_result(m, winner, duration, team_a_score, team_b_score);
	};

	deps.run_stage(state.current_stage_matches, commit);
}

void tournament_coordinator::update_skills() {
	if (!cfg.skill_by_match_time) {
		return;
	}

	for (const auto& m : state.current_stage_matches) {
		if (!m.result.has_value()) {
			continue;
		}

		state.teams[m.result->winner_team_index].skill = ::skill_from_match_time(m.result->duration_secs);
	}
}

void tournament_coordinator::eliminate_losers() {
	for (const auto& m : state.current_stage_matches) {
		if (!m.result.has_value()) {
			LOG("Tournament: match port=%x unresolved; nothing to eliminate.", m.port);
			continue;
		}

		const auto& r = *m.result;

		const uint32_t loser =
			r.winner_team_index == m.team_a_index ?
			m.team_b_index :
			m.team_a_index
		;

		state.teams[loser].eliminated = true;

		const bool winner_is_a = r.winner_team_index == m.team_a_index;

		tournament_match_history_entry h;
		h.played_in_stage = state.stage_index;
		h.winner_player_ids = state.teams[r.winner_team_index].player_ids;
		h.loser_player_ids = state.teams[loser].player_ids;
		h.duration_secs = r.duration_secs;
		h.winner_score = winner_is_a ? r.team_a_score : r.team_b_score;
		h.loser_score = winner_is_a ? r.team_b_score : r.team_a_score;

		state.match_history.push_back(std::move(h));
	}
}

void tournament_coordinator::advance_stage() {
	state.stage_index += 1;
	state.current_stage_matches.clear();
}

void tournament_coordinator::persist() const {
	state.save(state_path);
}

void tournament_coordinator::declare_winner() const {
	if (const auto winner = state.sole_surviving_team_index()) {
		const auto& t = state.teams[*winner];

		std::string roster;

		for (size_t i = 0; i < t.player_ids.size(); ++i) {
			if (i > 0) {
				roster += ", ";
			}

			roster += t.player_ids[i];
		}

		LOG("Tournament WINNER: [%x]", roster);
	}
	else {
		LOG("Tournament ended without a sole winner (interrupted?).");
	}
}

void tournament_coordinator::cleanup() const {
	const auto stamp = augs::date_time().get_readable_for_file_long();

	auto archive = [&stamp](const augs::path_type& src, const std::string& kind) {
		if (!augs::exists(src)) {
			return;
		}

		const auto archive_name = std::string("tournament.completed.") + stamp + "." + kind + ".json";
		const auto archive_path = src.parent_path() / archive_name;

		try {
			std::filesystem::rename(src, archive_path);
			LOG("Tournament: archived %x to %x.", kind, archive_path.string());
		}
		catch (const std::exception& err) {
			LOG("Tournament: failed to archive %x (%x); leaving in place.", kind, err.what());
		}
	};

	/*
		Snapshot both the live state and the config the tournament ran with under
		the same timestamp. The config is intentionally not deleted on failure -
		operators may have hand-edited it and a stale copy is more useful than
		nothing for forensics; the state file is the consumable runtime artifact
		and gets removed on rename failure to keep load() from picking it up.
	*/

	archive(state_path, "state");
	archive(config_path, "config");

	if (augs::exists(state_path)) {
		try {
			std::filesystem::remove(state_path);
		}
		catch (const std::exception& err) {
			LOG("Tournament: failed to remove state file: %x", err.what());
		}
	}
}

void tournament_coordinator::run() {
	initialize_state_if_fresh();
	persist();

	auto wipe_port = [this](const uint16_t port) {
		if (deps.wipe_recovery_for_port) {
			deps.wipe_recovery_for_port(port);
		}
	};

	while (!finished()) {
		if (deps.should_interrupt && deps.should_interrupt()) {
			LOG("Tournament: interrupted; state persisted, exiting.");
			return;
		}

		/*
			On a fresh stage we compute pairings, persist, and wipe every port the
			new stage will bind. That last step matters when stage N+1 has more
			matches than stage N (e.g. when stage 0 byes shrink the bracket to a
			pow2): the extra ports may carry an unrelated old snapshot from a prior
			tournament/run, and a server_setup constructed on them would otherwise
			resume that snapshot's arena.

			On a recovery (state file loaded with current_stage_matches already
			populated) we skip pair_teams so resume reuses the exact pairings,
			ports and rosters that were live before the crash. We only wipe ports
			of already-resolved matches in that case - per-port recovery files for
			unresolved matches ARE the mid-match resume payload we want load() to
			pick up.
		*/

		if (state.current_stage_matches.empty()) {
			pair_teams();
			persist();

			for (const auto& m : state.current_stage_matches) {
				wipe_port(m.port);
			}
		}
		else {
			LOG("Tournament: resuming stage %x with %x ongoing matches.", state.stage_index, state.current_stage_matches.size());

			for (const auto& m : state.current_stage_matches) {
				if (m.result.has_value()) {
					wipe_port(m.port);
				}
			}
		}

		run_current_stage();

		if (deps.should_interrupt && deps.should_interrupt()) {
			persist();
			return;
		}

		/*
			A match that finished the stage without resolving means the server worker
			exited before posting a match summary - a real bug somewhere downstream
			(e.g. yojimbo bind/socket issue we didn't catch, internal assertion).
			Don't silently advance: that would let both teams survive into the next
			stage and corrupt the bracket. Persist and bail loudly; operator can
			edit tournament.ongoing.json to force a resolution and resume.
		*/

		for (const auto& m : state.current_stage_matches) {
			if (!m.result.has_value()) {
				LOG(
					"Tournament: stage %x match on port %x (%x) exited without resolving. "
					"Aborting tournament; edit %x to force a resolution before resuming.",
					state.stage_index, m.port, m.server_name, state_path.string()
				);

				persist();
				return;
			}
		}

		update_skills();
		eliminate_losers();
		advance_stage();
		persist();

		/*
			Block until the per-port recovery worker has drained any pending writes
			or deletes from this stage. Without this, the next stage's server_setup
			on the same port could call load() before the worker's queued delete
			runs and resume the just-finished match's snapshot - wrong arena, wrong
			teams.
		*/

		if (deps.wait_for_pending_recovery_io) {
			deps.wait_for_pending_recovery_io();
		}
	}

	declare_winner();
	cleanup();
}

#endif
