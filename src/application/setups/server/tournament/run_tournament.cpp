#if HEADLESS
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

#include "application/setups/server/tournament/run_tournament.h"
#include "application/setups/server/tournament/tournament_config.h"
#include "application/setups/server/tournament/tournament_state.h"
#include "application/setups/server/tournament/tournament_coordinator.h"
#include "application/setups/server/server_setup.h"
#include "application/setups/server/server_assigned_teams.h"
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

tournament_config tournament_config::from_file(const augs::path_type& path) {
	return augs::from_json_file<tournament_config>(path);
}

work_result run_tournament(const run_tournament_input& in) {
	LOG("Tournament: loading config from %x", in.tournament_file_path.string());

	tournament_config cfg;

	try {
		cfg = tournament_config::from_file(in.tournament_file_path);
	}
	catch (const std::exception& err) {
		LOG("Tournament: failed to load config: %x", err.what());
		return work_result::FAILURE;
	}

	if (cfg.teams.size() < 2) {
		LOG("Tournament: need at least 2 teams (got %x). Aborting.", cfg.teams.size());
		return work_result::FAILURE;
	}

	if (cfg.maps.empty()) {
		LOG("Tournament: 'maps' must list at least one 'arena mode' entry. Aborting.");
		return work_result::FAILURE;
	}

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
		process as separate threads, just like the existing num_ranked_servers
		path. Each instance is configured with:
		    - vars.shutdown_after_one_match = true (graceful exit after summary intermission)
		    - vars.ranked.autostart_when = ALWAYS (start as soon as both teams join)
		    - per-match assigned_teams (team_a -> RESISTANCE, team_b -> METROPOLIS)
		    - per-match arena/mode from tournament_config.maps
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

		std::vector<std::thread> threads;
		threads.reserve(matches.size());

		for (auto& match : matches) {
			if (match.resolved) {
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
			this_config.server.ranked.autostart_when = ranked_autostart_type::ALWAYS;
			this_config.server.webrtc_udp_mux = true;
			this_config.server.webrtc_port_range_begin = webrtc_port_counter++;
			this_config.server.daily_autoupdate = false;

			/*
				Force overtime on tournament matches: the bracket has no machinery
				for replays, so a tie would silently eliminate one side at random.
				With overtime forced on, the mode plays out until a winner exists.
			*/

			this_config.server.allow_overtime = true;

			server_assigned_teams roster;

			auto fill_faction = [&roster](const std::vector<account_id_string>& ids, const faction_type f) {
				for (const auto& id : ids) {
					roster.id_to_faction[id] = f;
				}
			};

			fill_faction(match.team_a_player_ids, team_a_faction);
			fill_faction(match.team_b_player_ids, team_b_faction);
			fill_faction(match.spectator_player_ids, faction_type::SPECTATOR);

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
				this_config = std::move(this_config),
				roster = std::move(roster),
				stage_should_interrupt,
				on_instance_exit,
				commit,
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
						in.canon_config_with_confd.server,
						this_config.server_private,
						this_config.client,
						this_config.dedicated_server,
						true,
						roster,
						this_config.webrtc_signalling_server_url,
#if CRASH_RECOVERY
						in.recovery_worker,
#endif
						std::string("")
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

				server_ptr->on_match_summary = [match_ptr, commit](const messages::match_summary_message& s) {
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

					const auto roster_contains = [](const std::vector<account_id_string>& roster, const std::string& id) {
						for (const auto& r : roster) {
							if (r == id) {
								return true;
							}
						}

						return false;
					};

					const bool team_a_in_first = [&]() {
						for (const auto& winner_entry : s.first_faction) {
							if (roster_contains(match_ptr->team_a_player_ids, winner_entry.account_id)) {
								return true;
							}
						}

						return false;
					}();

					const bool team_b_in_first = [&]() {
						for (const auto& winner_entry : s.first_faction) {
							if (roster_contains(match_ptr->team_b_player_ids, winner_entry.account_id)) {
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

					const uint32_t winner_team_index = team_a_in_first ? match_ptr->team_a_index : match_ptr->team_b_index;

					commit(*match_ptr, winner_team_index, s.effective_playtime_secs);
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

	coordinator.run();

	return work_result::SUCCESS;
}
#endif
