#if HEADLESS
#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "augs/log.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/string/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/misc/secure_hash.h"

#include "application/setups/server/tournament/run_tournament.h"
#include "application/setups/server/tournament/tournament_config.h"
#include "application/setups/server/tournament/tournament_state.h"
#include "application/setups/server/tournament/tournament_coordinator.h"
#include "application/setups/server/server_setup.h"
#include "application/setups/server/server_assigned_teams.h"
#include "application/setups/server/server_nickname_auth.h"
#if CRASH_RECOVERY
#include "application/setups/server/crash_recovery/server_recovery_worker.h"
#endif
#include "application/main/self_updater.h"
#include "application/session_profiler.h"
#include "application/main/dedicated_server_worker.hpp"
#include "application/config_json_table.h"
#include "cmd_line_params.h"
#include "all_paths.h"
#include "game/enums/faction_type.h"
#include "game/messages/hud_message.h"

namespace {
	std::string trimmed(const std::string& s) {
		const auto first = s.find_first_not_of(" \t\r\n");

		if (first == std::string::npos) {
			return std::string();
		}

		return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
	}

	std::vector<std::string> split_list(const std::string& value, const std::string& separators) {
		std::vector<std::string> out;
		std::string current;

		auto flush = [&]() {
			if (const auto item = ::trimmed(current); !item.empty()) {
				out.push_back(item);
			}

			current.clear();
		};

		for (const auto c : value) {
			if (separators.find(c) != std::string::npos) {
				flush();
			}
			else {
				current += c;
			}
		}

		flush();
		return out;
	}

	std::optional<uint32_t> as_count(const std::string& value, const uint32_t min, const uint32_t max) {
		if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos) {
			return std::nullopt;
		}

		const auto n = static_cast<uint32_t>(std::atoi(value.c_str()));

		if (n < min || n > max) {
			return std::nullopt;
		}

		return n;
	}
}

/*
	--tournament <players> | --tournament-teams "alice, bob; carol, dave" (or one team per line),
	--tournament-team-size <n>, --tournament-arenas "de_cyberaqua,de_silo" - one per bracket round, the final last.
*/

tournament_config tournament_config::from_params(const cmd_line_params& params, std::string& error) {
	tournament_config out;

	if (!params.tournament_team_size.empty()) {
		if (const auto size = ::as_count(params.tournament_team_size, 1, 8)) {
			out.open_team_size = *size;
		}
		else {
			error = typesafe_sprintf("--tournament-team-size: expected a number from 1 to 8, got \"%x\"", params.tournament_team_size);
			return out;
		}
	}

	if (!params.tournament_teams.empty()) {
		for (const auto& line : ::split_list(params.tournament_teams, ";\n")) {
			out.teams.push_back(::split_list(line, ","));
		}

		if (out.teams.size() < 2) {
			error = "--tournament-teams: expected at least 2 teams, one per line or separated by ;";
			return out;
		}

		if (!params.tournament_team_size.empty()) {
			for (const auto& team : out.teams) {
				if (team.size() != out.open_team_size) {
					error = typesafe_sprintf("--tournament-teams: a team of %x players, expected %x (--tournament-team-size)", team.size(), out.open_team_size);
					return out;
				}
			}
		}
	}
	else if (const auto players = ::as_count(params.tournament_players, 2, 128)) {
		out.open_players = *players;
	}
	else {
		error = typesafe_sprintf("--tournament: expected a number of players from 2 to 128, got \"%x\"", params.tournament_players);
		return out;
	}

	out.arenas = ::split_list(params.tournament_arenas, ",");

	if (out.arenas.empty()) {
		/*
			As the config generator's default: de_cyberaqua and de_silo by turns.
		*/

		uint32_t bracket = 1;

		while (bracket < out.num_teams()) {
			bracket *= 2;
		}

		for (uint32_t round = 0; (1u << round) < bracket; ++round) {
			out.arenas.push_back(round % 2 == 0 ? "de_cyberaqua" : "de_silo");
		}
	}

	return out;
}

std::string tournament_config::compute_hash() const {
	const auto serialized = augs::to_json_string_nopretty(*this);
	return augs::bytes_to_hex(augs::secure_hash(serialized));
}

work_result run_tournament(const run_tournament_input& in) {
	auto cfg = in.config;

	LOG(
		"Tournament: %x, arenas: %x.",
		cfg.is_open() ? typesafe_sprintf("open first round for %x players, %x per team", cfg.open_players, cfg.open_team_size) : typesafe_sprintf("%x teams set up beforehand", cfg.teams.size()),
		cfg.arenas.size()
	);

	const auto base_port = in.config_pattern.server_start.port;

	tournament_coordinator_dependencies deps;
	deps.base_port = base_port;
	deps.should_interrupt = in.handle_sigint;

	deps.wipe_recovery_for_port = [&in](const uint16_t port) {
#if CRASH_RECOVERY
		const auto path = ::get_server_recovery_file_path(port);

		/*
			Drain the worker before deleting so a still-queued write for the same
			path can't materialize a file we just removed. Coordinator already
			drains between stages, but call sites that invoke this lambda outside
			that flow (e.g. stage-0 fresh wipe) need it locally.
		*/

		in.recovery_worker.wait_until_idle();

		try {
			if (augs::exists(path)) {
				std::filesystem::remove(path);
				LOG("Tournament: wiped stale recovery file %x", path.string());
			}
		}
		catch (const std::exception& err) {
			LOG("Tournament: failed to remove %x: %x", path.string(), err.what());
		}
#else
		(void)port;
#endif
	};

#if CRASH_RECOVERY
	deps.wait_for_pending_recovery_io = [&in]() {
		in.recovery_worker.wait_until_idle();
	};
#endif

	/*
		The per-stage runner. Builds one server_setup per match, all in this
		process as separate threads, just like the ranked instances.
		Each instance is a TOURNAMENT one (ranked, label t1, t2...) with the ranked vars and:
		    - vars.shutdown_after_one_match = true (graceful exit after summary intermission)
		    - per-match assigned_teams (team_a -> RESISTANCE, team_b -> METROPOLIS)
		    - per-match arena/mode from tournament_config.arenas
		    - per-match server_name from coordinator
		    - on_match_summary callback that writes back into the match struct
	*/

	constexpr auto team_a_faction = faction_type::RESISTANCE;
	constexpr auto team_b_faction = faction_type::METROPOLIS;

	deps.run_stage = [&](
		std::vector<tournament_match>& matches,
		tournament_commit_match_result_fn commit
	) {
		std::atomic<bool> any_failure = false;

		auto stage_should_interrupt = [&in]() {
			return in.handle_sigint && in.handle_sigint();
		};

		auto on_instance_exit = [&any_failure](const work_result this_result) {
			if (this_result != work_result::SUCCESS) {
				any_failure.store(true);
			}
		};

		uint16_t webrtc_port_counter = in.config_pattern.server.webrtc_port_range_begin;

		/*
			Close round 1 from the administration panel of any open match sets it for all of them.
		*/

		const auto close_open_round = std::make_shared<std::atomic<bool>>(false);
		const auto open_round_players = std::make_shared<std::atomic<uint32_t>>(0);

		std::vector<std::thread> threads;
		threads.reserve(matches.size());

		/*
			Parallel matches are servers of their own: the host's key, each its label - t1, t2...
		*/

		uint16_t match_index = 0;

		for (auto& match : matches) {
			const auto this_match_index = match_index++;

			if (match.result.has_value()) {
				/*
					Carried over from a previous (crashed) session: this match already
					concluded and was persisted. No instance to spawn. The coordinator
					owns wiping the per-port recovery file (already done above its call
					to run_stage), so nothing to do here.
				*/

				continue;
			}

			auto this_config = in.config_pattern;

			this_config.server_start.port = match.port;
			this_config.server.arena = ::get_first_word(match.arena_and_mode);
			this_config.server.game_mode = ::get_second_word(match.arena_and_mode);
			this_config.server.server_name = match.server_name;
			this_config.server.shutdown_after_one_match = true;

			auto canon_server_vars = in.canon_config_with_confd.server;
			this_config.server.webrtc_udp_mux = true;
			this_config.server.webrtc_port_range_begin = webrtc_port_counter++;
			this_config.server.daily_autoupdate = false;

			if (match.open_team_size > 0) {
				/*
					Full once the pair is there - with a few places for the admins watching.
				*/

				this_config.server_start.slots = static_cast<int>(2 * match.open_team_size + 4);
			}

			/*
				Force overtime on tournament matches: the bracket has no machinery
				for replays, so a tie would silently eliminate one side at random.
				With overtime forced on, the mode plays out until a winner exists.
			*/

			this_config.server.allow_overtime = true;

			server_assigned_teams roster;

			/*
				Rosters of account ids (steam_..., key_..., crazygames_...) are matched against the players' certs.
				Rosters of plain nicknames fall back to nickname auth, so LAN tournaments work out of the box -
				operators don't have to remember to also flip authenticate_with_nicknames in the dedicated-server config.
			*/

			{
				auto is_account_id = [](const account_id_string& id) {
					for (const auto prefix : { "steam_", "key_", "crazygames_" }) {
						if (id.rfind(prefix, 0) == 0) {
							return true;
						}
					}

					return false;
				};

				auto all_account_ids = [&](const std::vector<account_id_string>& ids) {
					return std::all_of(ids.begin(), ids.end(), is_account_id);
				};

				const bool rosters_are_accounts =
					all_account_ids(match.team_a_player_ids)
					&& all_account_ids(match.team_b_player_ids)
					&& all_account_ids(match.spectator_player_ids)
				;

				if (!rosters_are_accounts) {
					this_config.server.authenticate_with_nicknames = true;

					auto any_account_id = [&](const std::vector<account_id_string>& ids) {
						return std::any_of(ids.begin(), ids.end(), is_account_id);
					};

					const bool mixed =
						any_account_id(match.team_a_player_ids)
						|| any_account_id(match.team_b_player_ids)
						|| any_account_id(match.spectator_player_ids)
					;

					if (mixed) {
						LOG("WARNING! Tournament match on port %x mixes account ids with nicknames - all are treated as nicknames.", match.port);
					}
				}
				else if (this_config.server.authenticate_with_nicknames) {
					/*
						Nickname auth would make the players "nick_..." - no account id in the roster could match.
					*/

					LOG("Tournament match on port %x: the roster is account ids - authenticate_with_nicknames turned off.", match.port);
					this_config.server.authenticate_with_nicknames = false;
				}

				LOG(
					"Tournament match on port %x authenticates players by %x.",
					match.port,
					this_config.server.authenticate_with_nicknames ? "nickname" : "account"
				);
			}

			/*
				The tournament JSON holds raw nicknames ("alice"), but when
				authenticate_with_nicknames is set the server authenticates clients
				as "nick_alice". Roster keys and summary matching must use that same
				authenticated form or every client is kicked as "wrong match".
			*/

			const bool nickname_auth = this_config.server.authenticate_with_nicknames;

			auto to_auth_id = [nickname_auth](const account_id_string& id) {
				return nickname_auth ? nickname_to_account_id(id) : id;
			};

			auto fill_faction = [&roster, &to_auth_id](const std::vector<account_id_string>& ids, const faction_type f) {
				for (const auto& id : ids) {
					roster.id_to_faction[to_auth_id(id)] = f;
				}
			};

			fill_faction(match.team_a_player_ids, team_a_faction);
			fill_faction(match.team_b_player_ids, team_b_faction);
			fill_faction(match.spectator_player_ids, faction_type::SPECTATOR);

			roster.open_team_size = match.open_team_size;

			auto* match_ptr = &match;

			/*
				Each match owns its own thread end-to-end: bind retry, server_setup
				construction, callbacks, and the dedicated_server_worker loop all
				run inside it. Doing it this way means a hung bind on one match's
				port doesn't keep the other matches in the stage from launching -
				they bind concurrently.
			*/

			threads.emplace_back([
				match_ptr,
				this_match_index,
				this_config = std::move(this_config),
				canon_server_vars = std::move(canon_server_vars),
				roster = std::move(roster),
				nickname_auth,
				stage_should_interrupt,
				on_instance_exit,
				commit,
				close_open_round,
				open_round_players,
				expected_players = in.config.open_players,
				&in
			]() {
				LOG_THREAD_PREFFIX() = typesafe_sprintf("[T%x] ", match_ptr->port);

				/*
					yojimbo's Server.Start() doesn't surface a return code; on bind failure
					the adapter just reports !is_running() and the worker loop exits
					immediately, silently leaving the match unresolved (and the bracket
					stuck - no resolution means no elimination). Retry indefinitely with a
					short backoff so the operator has time to kill any conflicting process
					without restarting the whole tournament. SIGINT breaks the loop.

					The port itself stays the same across retries so per-port crash-recovery
					semantics aren't disturbed.
				*/

				constexpr auto bind_retry_delay = std::chrono::seconds(2);

				std::unique_ptr<server_setup> server_ptr;

				for (int attempt = 1; ; ++attempt) {
					LOG(
						"Starting tournament match instance on port %x (attempt %x). Match: %x",
						match_ptr->port, attempt, match_ptr->server_name
					);

					auto candidate = std::make_unique<server_setup>(
						in.official,
						this_config.server_start,
						this_config.server,
						canon_server_vars,
						this_config.server_private,
						this_config.client,
						this_config.dedicated_server,
						true,
						roster,
						this_config.webrtc_signalling_server_url,
#if CRASH_RECOVERY
						in.recovery_worker,
#endif
						std::string(""),
						server_temp_var_overrides(),
						server_instance_type::TOURNAMENT,
						static_cast<uint16_t>(this_match_index + 1)
					);

					if (candidate->is_running()) {
						server_ptr = std::move(candidate);
						break;
					}

					LOG("Tournament: port %x bind failed (attempt %x). Retrying in %xs (Ctrl+C to abort).",
						match_ptr->port, attempt, static_cast<int>(bind_retry_delay.count()));

					/*
						candidate goes out of scope here, destructing the failed adapter
						and releasing whatever partial socket state it held.
					*/

					if (stage_should_interrupt()) {
						LOG("Tournament: interrupted while waiting for port %x.", match_ptr->port);
						break;
					}

					std::this_thread::sleep_for(bind_retry_delay);
				}

				if (!server_ptr) {
					return;
				}

				server_ptr->heartbeat_reported_tournament_stage = static_cast<ranked_server_type>(match_ptr->heartbeat_ranked_type);

				/*
					An open match's teams are whoever took the places - read from its roster once it's decided.
				*/

				const auto* const server_raw = server_ptr.get();

				auto take_open_teams = [server_raw](const server_assigned_teams& roster, tournament_match_outcome& out) {
					for (const auto& [id, faction] : roster.id_to_faction) {
						if (faction == team_a_faction) {
							out.open_team_a.push_back(id);
						}
						else if (faction == team_b_faction) {
							out.open_team_b.push_back(id);
						}
					}

					std::sort(out.open_team_a.begin(), out.open_team_a.end());
					std::sort(out.open_team_b.begin(), out.open_team_b.end());

					for (const auto& id : out.open_team_a) {
						out.open_names_a.push_back(server_raw->find_nickname_of_account(id));
					}

					for (const auto& id : out.open_team_b) {
						out.open_names_b.push_back(server_raw->find_nickname_of_account(id));
					}
				};

				if (match_ptr->open_team_size > 0) {
					server_ptr->close_open_round = close_open_round;
					server_ptr->open_round_players = open_round_players;
					server_ptr->open_round_expected_players = expected_players;

					/*
						Closed before its pair was there: whoever waited advances without playing - all of them as one
						team, whichever side they took. Nobody: an empty server, gone.
					*/

					server_ptr->on_open_match_closed = [match_ptr, commit, take_open_teams](const server_assigned_teams& roster) {
						auto outcome = tournament_match_outcome();
						outcome.walkover = true;

						take_open_teams(roster, outcome);

						auto& waiting = outcome.open_team_a;
						waiting.insert(waiting.end(), outcome.open_team_b.begin(), outcome.open_team_b.end());
						outcome.open_team_b.clear();

						auto& waiting_names = outcome.open_names_a;
						waiting_names.insert(waiting_names.end(), outcome.open_names_b.begin(), outcome.open_names_b.end());
						outcome.open_names_b.clear();

						commit(*match_ptr, outcome);
					};
				}

				server_ptr->on_match_summary = [match_ptr, commit, nickname_auth, server_raw, take_open_teams](const messages::match_summary_message& s) {
					auto outcome = tournament_match_outcome();

					if (match_ptr->open_team_size > 0 && match_ptr->team_a_index == no_tournament_team_v) {
						take_open_teams(server_raw->get_assigned_teams(), outcome);
					}

					const auto& team_a_ids = match_ptr->open_team_size > 0 ? outcome.open_team_a : match_ptr->team_a_player_ids;
					const auto& team_b_ids = match_ptr->open_team_size > 0 ? outcome.open_team_b : match_ptr->team_b_player_ids;

					/*
						Faction of `first_faction` shifts after a halftime swap (a player who
						started on RESISTANCE can be on METROPOLIS by match end), so the
						faction enum is unreliable. Match by account_id, which is the same key
						server_assigned_teams uses to bucket players - identical to what the
						tournament JSON's team rosters must contain.

						Players who abandoned the match still appear in their faction's entry
						with abandoned_at_score set (server_setup's ranked-flow guarantee), so
						an abandon by a whole team simply leaves the surviving team as the
						sole occupant of first_faction and is detected as a normal win.
					*/

					const auto roster_contains = [nickname_auth](const std::vector<account_id_string>& roster, const std::string& id) {
						for (const auto& r : roster) {
							const auto auth_r = nickname_auth ? nickname_to_account_id(r) : r;

							if (auth_r == id) {
								return true;
							}
						}

						return false;
					};

					const bool team_a_in_first = [&]() {
						for (const auto& winner_entry : s.first_faction) {
							if (roster_contains(team_a_ids, winner_entry.account_id)) {
								return true;
							}
						}

						return false;
					}();

					const bool team_b_in_first = [&]() {
						for (const auto& winner_entry : s.first_faction) {
							if (roster_contains(team_b_ids, winner_entry.account_id)) {
								return true;
							}
						}

						return false;
					}();

					if (!team_a_in_first && !team_b_in_first) {
						LOG(
							"Tournament: no roster id found in first_faction for port %x. "
							"Coordinator will treat the match as unresolved and abort the tournament.",
							match_ptr->port
						);

						return;
					}

					if (team_a_in_first && team_b_in_first) {
						LOG(
							"Tournament: both rosters appear in first_faction for port %x (shouldn't happen). "
							"Defaulting to team_a.",
							match_ptr->port
						);
					}

					/*
						Winner is whichever roster lives in s.first_faction. arena_mode sets
						first_faction to the surviving side when losers_abandoned, regardless
						of which side was leading. So a 4:1 game where the leading team
						abandons correctly resolves as: surviving (score=1) wins, abandoning
						(score=4) is eliminated. The per-team scores are preserved as-is, so
						the recorded result can legitimately have winner_score < loser_score.
					*/

					outcome.team_a_won = team_a_in_first;
					outcome.duration_secs = s.effective_playtime_secs;
					outcome.team_a_score = static_cast<uint32_t>(std::max(0, team_a_in_first ? s.first_team_score : s.second_team_score));
					outcome.team_b_score = static_cast<uint32_t>(std::max(0, team_a_in_first ? s.second_team_score : s.first_team_score));

					commit(*match_ptr, outcome);
				};

				auto worker_in = dedicated_server_worker_input {
					std::move(server_ptr),
					in.params.appimage_path,
					this_config.self_update,
					nullptr,
					typesafe_sprintf("tournament match #%x", match_ptr->port),
					typesafe_sprintf("[T%x] ", match_ptr->port)
				};

				on_instance_exit(::dedicated_server_worker(worker_in, stage_should_interrupt));
			});
		}

		for (auto& t : threads) {
			t.join();
		}

		if (any_failure.load()) {
			LOG("Tournament: at least one instance exited with non-SUCCESS.");
		}
	};

	auto coordinator = tournament_coordinator(
		std::move(cfg),
		in.state_file_path,
		std::move(deps)
	);

	if (!coordinator.run()) {
		/*
			Played to the end already. Stay up doing nothing rather than quit - a container would
			only be started again and again (restart: unless-stopped).
		*/

		while (!(in.handle_sigint && in.handle_sigint())) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	return work_result::SUCCESS;
}
#endif
