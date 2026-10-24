#if HEADLESS
#include <algorithm>
#include <filesystem>
#include "application/setups/server/tournament/tournament_coordinator.h"
#include "application/masterserver/server_heartbeat.h"
#include "application/network/account_id.h"
#include "augs/log.h"
#include "augs/misc/randomization.h"
#include "augs/misc/date_time.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite.h"

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
	augs::path_type in_state_path,
	tournament_coordinator_dependencies in_deps
) :
	cfg(std::move(in_cfg)),
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
			resumed = true;
			LOG("Tournament: resumed from %x at stage %x.", state_path.string(), state.stage_index);
		}
	}

	state.config_hash = current_hash;
}

bool tournament_coordinator::finished() const {
	/*
		The open first round has no teams yet - they come from its matches.
	*/

	if (cfg.is_open() && state.stage_index == 0) {
		return false;
	}

	return state.tournament_finished();
}

/*
	The archived state of this very tournament (the same hash of its flags), if it was played to the end here.
*/

std::optional<augs::path_type> tournament_coordinator::find_completed_archive() const {
	const auto dir = state_path.parent_path();
	const auto current_hash = cfg.compute_hash();

	try {
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			const auto name = entry.path().filename().string();

			if (name.rfind("tournament.completed.", 0) != 0 || name.find(".state.json") == std::string::npos) {
				continue;
			}

			if (const auto archived = tournament_state::load(entry.path()); archived && archived->config_hash == current_hash) {
				return entry.path();
			}
		}
	}
	catch (const std::exception& err) {
		LOG("Tournament: could not look for the finished tournaments in %x: %x", dir.string(), err.what());
	}

	return std::nullopt;
}

uint32_t tournament_coordinator::add_team_of(const std::vector<account_id_string>& player_ids, const std::vector<std::string>& nicknames) {
	tournament_team_state t;
	t.player_ids = player_ids;
	t.nicknames = nicknames;
	state.teams.push_back(std::move(t));

	return static_cast<uint32_t>(state.teams.size() - 1);
}

void tournament_coordinator::initialize_state_if_fresh() {
	if (state.initialized) {
		return;
	}

	if (cfg.is_open()) {
		state.stage_index = 0;
		state.initialized = true;

		LOG("Tournament: an open first round for %x players, %x per team.", cfg.open_players, cfg.open_team_size);
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

		/*
			A key copied from the game is "0x<hex>" - written so, it would not count as an account id
			and the whole match would fall back to nickname auth.
		*/

		for (const auto& written : cfg.teams[team_idx]) {
			ts.player_ids.push_back(::normalize_account_id(written));
		}

		/*
			Listed with account ids: shown by the nicknames written next to them.
		*/

		if (team_idx < cfg.team_nicknames.size()) {
			ts.nicknames = cfg.team_nicknames[team_idx];
		}

		ts.skill = static_cast<float>(N - rank);
		ts.eliminated = false;

		state.teams.push_back(std::move(ts));
	}

	state.stage_index = 0;
	state.initialized = true;

	LOG("Tournament: initialized with %x teams.", N);
}

/*
	A server per pair of the bracket: 16 players 1v1 - 8 servers, 5 players - 4 (the bracket of 8).
	Whoever waits alone on one when round 1 is closed advances without playing.
*/

void tournament_coordinator::pair_open_first_round() {
	const auto bracket = next_pow2(std::max(2u, cfg.num_teams()));
	const auto num_servers = bracket / 2;

	uint16_t port_cursor = deps.base_port;

	for (uint32_t i = 0; i < num_servers; ++i) {
		tournament_match m;
		m.team_a_index = no_tournament_team_v;
		m.team_b_index = no_tournament_team_v;
		m.open_team_size = cfg.open_team_size;
		m.port = port_cursor++;
		m.arena_and_mode = cfg.arena_for_rounds_left(tournament_config::rounds_for_teams(cfg.num_teams()));
		m.heartbeat_ranked_type = static_cast<uint8_t>(bracket == 2 ? ranked_server_type::TOURNAMENT_FINALS : ranked_server_type::TOURNAMENT_MID);
		m.server_name = typesafe_sprintf("Match #%x (any pair)", i + 1);

		state.current_stage_matches.push_back(std::move(m));
	}

	LOG("Tournament: round 1 open - %x servers, the first ones there make each match.", num_servers);
}

void tournament_coordinator::pair_teams() {
	state.current_stage_matches.clear();

	if (cfg.is_open() && state.stage_index == 0) {
		pair_open_first_round();
		return;
	}

	auto alive = state.surviving_team_indices();

	std::sort(alive.begin(), alive.end(), [this](const uint32_t a, const uint32_t b) {
		return state.teams[a].skill > state.teams[b].skill;
	});

	std::vector<uint32_t> playing = alive;
	std::vector<uint32_t> getting_bye;

	/*
		Byes whenever the teams left aren't 2, 4, 8...: in the first round of listed teams,
		or after an open first round that fewer came to.
	*/

	if (const auto N = static_cast<uint32_t>(alive.size()); next_pow2(N) != N) {
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

			LOG("Tournament stage %x: %x byes (to %x).", state.stage_index, byes, cfg.byes_to_strongest ? "strongest" : "weakest");

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

	/*
		Counted from the end of the list: the final always on the last arena, whatever number came.
	*/

	const std::string& arena_and_mode_for_stage = cfg.arena_for_rounds_left(tournament_config::rounds_for_teams(static_cast<uint32_t>(alive.size())));

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
		shared_spectator_ids.push_back(::normalize_account_id(pid));
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
	/*
		By the nicknames, if known - a team from an open first round is account ids otherwise.
	*/

	auto stringify = [](const tournament_team_state& t) {
		std::string s = "[";

		for (size_t i = 0; i < t.player_ids.size(); ++i) {
			if (i > 0) {
				s += ", ";
			}

			const bool has_nickname = i < t.nicknames.size() && !t.nicknames[i].empty();
			s += has_nickname ? t.nicknames[i] : t.player_ids[i];
		}

		s += "]";
		return s;
	};

	return stringify(state.teams[m.team_a_index]) + " vs " + stringify(state.teams[m.team_b_index]);
}

void tournament_coordinator::commit_match_result(tournament_match& m, const tournament_match_outcome& outcome) {
	std::scoped_lock lk(state_lk);

	if (m.result.has_value() || m.closed_empty) {
		return;
	}

	/*
		Nobody came to a match of known teams before the join timeout: both are out (eliminate_losers).
	*/

	if (outcome.nobody_came && m.team_a_index != no_tournament_team_v) {
		m.closed_empty = true;
		LOG("Tournament: nobody came to the match on port %x (%x) - both teams are out.", m.port, m.server_name);

		state.save(state_path);

		if (deps.on_state_changed) {
			deps.on_state_changed(state);
		}

		return;
	}

	if (m.open_team_size > 0 && m.team_a_index == no_tournament_team_v) {
		/*
			An open match: its teams are whoever played it - or whoever waited on it when round 1 was closed.
		*/

		m.team_a_player_ids = outcome.open_team_a;
		m.team_b_player_ids = outcome.open_team_b;

		if (m.team_a_player_ids.empty() && m.team_b_player_ids.empty()) {
			m.closed_empty = true;
			LOG("Tournament: open match on port %x closed with nobody on it.", m.port);

			state.save(state_path);

			if (deps.on_state_changed) {
				deps.on_state_changed(state);
			}

			return;
		}

		m.team_a_index = add_team_of(m.team_a_player_ids, outcome.open_names_a);

		if (!m.team_b_player_ids.empty()) {
			m.team_b_index = add_team_of(m.team_b_player_ids, outcome.open_names_b);
		}
	}

	/*
		Teams listed by account ids get their nicknames from the match they played - for the live page.
	*/

	auto name_team = [&](const uint32_t team, const std::vector<std::string>& names) {
		if (team < state.teams.size() && state.teams[team].nicknames.empty() && names.size() == state.teams[team].player_ids.size()) {
			state.teams[team].nicknames = names;
		}
	};

	name_team(m.team_a_index, outcome.open_names_a);
	name_team(m.team_b_index, outcome.open_names_b);

	tournament_match_result r;
	r.winner_team_index = outcome.team_a_won ? m.team_a_index : m.team_b_index;
	r.duration_secs = outcome.duration_secs;
	r.team_a_score = outcome.team_a_score;
	r.team_b_score = outcome.team_b_score;
	r.walkover = outcome.walkover;

	m.result = r;

	state.save(state_path);

	if (deps.on_state_changed) {
		deps.on_state_changed(state);
	}
}

void tournament_coordinator::run_current_stage() {
	if (state.current_stage_matches.empty()) {
		return;
	}

	LOG("Tournament stage %x: running %x matches.", state.stage_index, state.current_stage_matches.size());

	auto commit = [this](tournament_match& m, const tournament_match_outcome& outcome) {
		commit_match_result(m, outcome);
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

		/*
			A walkover - nobody came to play them: the weakest, so the byes of the next round go to the teams that played.
		*/

		state.teams[m.result->winner_team_index].skill = m.result->walkover ? 0.0f : ::skill_from_match_time(m.result->duration_secs);
	}
}

void tournament_coordinator::eliminate_losers() {
	for (const auto& m : state.current_stage_matches) {
		if (m.closed_empty) {
			/*
				A match of known teams nobody came to: both out. An open one nobody came to had no teams.
			*/

			for (const auto team : { m.team_a_index, m.team_b_index }) {
				if (team < state.teams.size()) {
					state.teams[team].eliminated = true;
				}
			}

			continue;
		}

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

		if (loser == no_tournament_team_v) {
			/*
				A walkover: whoever waited alone goes on, nobody's eliminated.
			*/

			tournament_match_history_entry h;
			h.played_in_stage = state.stage_index;
			h.winner_player_ids = state.teams[r.winner_team_index].player_ids;
			h.walkover = true;

			state.match_history.push_back(std::move(h));
			continue;
		}

		state.teams[loser].eliminated = true;

		const bool winner_is_a = r.winner_team_index == m.team_a_index;

		tournament_match_history_entry h;
		h.played_in_stage = state.stage_index;
		h.winner_player_ids = state.teams[r.winner_team_index].player_ids;
		h.loser_player_ids = state.teams[loser].player_ids;
		h.duration_secs = r.duration_secs;
		h.winner_score = winner_is_a ? r.team_a_score : r.team_b_score;
		h.loser_score = winner_is_a ? r.team_b_score : r.team_a_score;

		/*
			The join timeout's walkover of known teams: the other one didn't come.
		*/

		h.walkover = r.walkover;

		state.match_history.push_back(std::move(h));
	}
}

void tournament_coordinator::advance_stage() {
	state.stage_index += 1;
	state.current_stage_matches.clear();
}

void tournament_coordinator::persist() const {
	state.save(state_path);

	if (deps.on_state_changed) {
		deps.on_state_changed(state);
	}
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
		LOG("Tournament over without a winner - nobody came to the last match.");
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
		The state, and the tournament as the flags described it, under the same timestamp.
		The state is what tells that this tournament was played to the end (find_completed_archive);
		removed if it can't be renamed, so load() doesn't pick it up again.
	*/

	archive(state_path, "state");

	try {
		const auto config_archive = state_path.parent_path() / (std::string("tournament.completed.") + stamp + ".config.json");
		augs::save_as_json(cfg, config_archive);
	}
	catch (const std::exception& err) {
		LOG("Tournament: failed to save the tournament's definition: %x", err.what());
	}

	if (augs::exists(state_path)) {
		try {
			std::filesystem::remove(state_path);
		}
		catch (const std::exception& err) {
			LOG("Tournament: failed to remove state file: %x", err.what());
		}
	}
}

bool tournament_coordinator::run() {
	if (!resumed) {
		if (const auto archive = find_completed_archive()) {
			completed_archive = *archive;

			LOG(
				"Tournament: this tournament was played to the end already - the results are in %x. "
				"It doesn't start again with the same settings; remove that file to play it once more.",
				archive->string()
			);

			return false;
		}
	}

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
			return true;
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
			return true;
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
			if (!m.result.has_value() && !m.closed_empty) {
				LOG(
					"Tournament: stage %x match on port %x (%x) exited without resolving. "
					"Aborting tournament; edit %x to force a resolution before resuming.",
					state.stage_index, m.port, m.server_name, state_path.string()
				);

				persist();
				aborted = true;
				return true;
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

	return true;
}


#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

namespace {
	/*
		An open tournament played by a script: every stage's matches resolved as `play` says - a stage's
		matches seen by the test as they were run.
	*/

	struct scripted_tournament {
		std::vector<std::vector<tournament_match>> stages;

		void run(
			tournament_config cfg,
			std::function<void(std::size_t stage, std::vector<tournament_match>&, tournament_commit_match_result_fn)> play
		) {
			/*
				Unit tests run at every start, servers' too: the coordinator's lines muted - never read as a tournament.
			*/

			LOG_THREAD_MUTED() = true;

			static uint32_t run_counter = 0;

			const auto dir = std::filesystem::temp_directory_path() / typesafe_sprintf(
				"hypersomnia-tournament-test-%x-%x",
				static_cast<uint64_t>(augs::date_time::secs_since_epoch() * 1000.0),
				run_counter++
			);

			std::filesystem::create_directories(dir);

			tournament_coordinator_dependencies deps;
			deps.base_port = 30000;

			deps.run_stage = [&](std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
				play(stages.size(), matches, commit);
				stages.push_back(matches);
			};

			auto coordinator = tournament_coordinator(std::move(cfg), dir / "tournament.ongoing.json", std::move(deps));
			coordinator.run();

			std::error_code ec;
			std::filesystem::remove_all(dir, ec);

			LOG_THREAD_MUTED() = false;
		}
	};

	tournament_match_outcome open_match(const std::vector<account_id_string>& a, const std::vector<account_id_string>& b, const float secs) {
		auto out = tournament_match_outcome();
		out.open_team_a = a;
		out.open_team_b = b;
		out.open_names_a = a;
		out.open_names_b = b;
		out.duration_secs = secs;
		out.walkover = b.empty();
		return out;
	}

	tournament_config open_config(const uint32_t players) {
		auto cfg = tournament_config();
		cfg.open_players = players;
		cfg.arenas = { "a1", "a2", "a3" };
		return cfg;
	}

	auto team_a_wins = [](std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
		for (auto& m : matches) {
			auto outcome = tournament_match_outcome();
			outcome.duration_secs = 100;
			commit(m, outcome);
		}
	};
}

TEST_CASE("Tournament: a walkover doesn't take the next round's bye") {
	auto t = scripted_tournament();

	t.run(open_config(8), [](const std::size_t stage, std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
		if (stage == 0) {
			REQUIRE(matches.size() == 4);

			commit(matches[0], open_match({ "A" }, { "B" }, 100));
			commit(matches[1], open_match({ "D" }, { "E" }, 50));
			commit(matches[2], open_match({ "C" }, {}, 0));
			commit(matches[3], open_match({}, {}, 0));
			return;
		}

		team_a_wins(matches, commit);
	});

	REQUIRE(t.stages.size() == 3);

	/*
		Round 2: A, C and D - the bye to D, the fastest winner; C, who waited alone, plays.
	*/

	const auto& round_2 = t.stages[1];
	REQUIRE(round_2.size() == 1);

	auto ids = round_2[0].team_a_player_ids;
	ids.insert(ids.end(), round_2[0].team_b_player_ids.begin(), round_2[0].team_b_player_ids.end());
	std::sort(ids.begin(), ids.end());

	REQUIRE(ids == std::vector<account_id_string> { "A", "C" });
}

TEST_CASE("Tournament: the final on the last arena when fewer came") {
	auto t = scripted_tournament();

	t.run(open_config(8), [](const std::size_t stage, std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
		if (stage == 0) {
			REQUIRE(matches[0].arena_and_mode == "a1");

			commit(matches[0], open_match({ "A" }, { "B" }, 100));
			commit(matches[1], open_match({ "C" }, {}, 0));
			commit(matches[2], open_match({}, {}, 0));
			commit(matches[3], open_match({}, {}, 0));
			return;
		}

		team_a_wins(matches, commit);
	});

	/*
		3 came of 8: A and C left after round 1 - round 2 is the final, on the arena chosen for the final.
	*/

	REQUIRE(t.stages.size() == 2);
	REQUIRE(t.stages[1].size() == 1);
	REQUIRE(t.stages[1][0].arena_and_mode == "a3");
}

TEST_CASE("Tournament: join timeout - nobody came, both teams are out") {
	auto cfg = tournament_config();
	cfg.teams = { { "A" }, { "B" }, { "C" }, { "D" } };
	cfg.arenas = { "a1", "a2" };

	auto t = scripted_tournament();

	t.run(cfg, [](const std::size_t stage, std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
		if (stage == 0) {
			REQUIRE(matches.size() == 2);

			auto nobody = tournament_match_outcome();
			nobody.walkover = true;
			nobody.nobody_came = true;
			commit(matches[0], nobody);

			auto a_came = tournament_match_outcome();
			a_came.walkover = true;
			a_came.team_a_won = true;
			commit(matches[1], a_came);
			return;
		}

		team_a_wins(matches, commit);
	});

	/*
		One match nobody came to, the other one's team that came wins: it's the only one left - no final to play.
	*/

	REQUIRE(t.stages.size() == 1);
}

TEST_CASE("Tournament: join timeout - nobody came to the final, no winner") {
	auto cfg = tournament_config();
	cfg.teams = { { "A" }, { "B" } };
	cfg.arenas = { "a1" };

	auto t = scripted_tournament();

	t.run(cfg, [](const std::size_t, std::vector<tournament_match>& matches, tournament_commit_match_result_fn commit) {
		auto nobody = tournament_match_outcome();
		nobody.walkover = true;
		nobody.nobody_came = true;
		commit(matches[0], nobody);
	});

	REQUIRE(t.stages.size() == 1);
}

TEST_CASE("Tournament: rounds and arenas counted from the end") {
	auto cfg = open_config(8);

	REQUIRE(tournament_config::rounds_for_teams(2) == 1);
	REQUIRE(tournament_config::rounds_for_teams(3) == 2);
	REQUIRE(tournament_config::rounds_for_teams(4) == 2);
	REQUIRE(tournament_config::rounds_for_teams(5) == 3);
	REQUIRE(tournament_config::rounds_for_teams(8) == 3);

	REQUIRE(cfg.arena_for_rounds_left(3) == "a1");
	REQUIRE(cfg.arena_for_rounds_left(1) == "a3");
	REQUIRE(cfg.arena_for_rounds_left(5) == "a1");

	cfg.arenas = { "only" };
	REQUIRE(cfg.arena_for_rounds_left(4) == "only");
}
#endif

#endif
