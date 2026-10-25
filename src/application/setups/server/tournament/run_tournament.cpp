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
#include "augs/templates/container_templates.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/misc/secure_hash.h"

#include "application/setups/server/tournament/run_tournament.h"
#include "application/setups/server/tournament/tournament_config.h"
#include "application/setups/server/tournament/tournament_state.h"
#include "application/setups/server/tournament/tournament_coordinator.h"
#include "application/setups/server/tournament/tournament_page.h"
#include "application/setups/server/tournament/tournament_page_server.h"
#include "3rdparty/include_httplib.h"
#include "application/setups/server/server_setup.h"
#include "application/setups/server/server_assigned_teams.h"
#include "application/network/account_id.h"
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
		if (value.empty() || value.size() > 9 || value.find_first_not_of("0123456789") != std::string::npos) {
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

	if (!params.tournament_teams.empty() && !params.tournament_players.empty()) {
		error = "--tournament and --tournament-teams (HYPERSOMNIA_TOURNAMENT and HYPERSOMNIA_TOURNAMENT_TEAMS) are given both - keep one";
		return out;
	}

	if (!params.tournament_teams.empty()) {
		/*
			"alice=0x8aed..., bob=steam_7656...; carol=..." - every player with an account id, or none of them:
			a list with some let in by their nicknames only would quietly let anyone in as them.
		*/

		std::size_t with_ids = 0;
		std::size_t players = 0;
		std::vector<std::string> seen_names;
		std::vector<std::string> seen_ids;

		for (const auto& line : ::split_list(params.tournament_teams, ";\n")) {
			std::vector<std::string> ids;
			std::vector<std::string> names;

			for (const auto& entry : ::split_list(line, ",")) {
				++players;

				const auto eq = entry.find('=');
				const auto name = ::trimmed(entry.substr(0, eq));

				if (name.empty()) {
					error = typesafe_sprintf("--tournament-teams: a player with no nickname: \"%x\"", entry);
					return out;
				}

				if (std::find(seen_names.begin(), seen_names.end(), name) != seen_names.end()) {
					error = typesafe_sprintf("--tournament-teams: %x is listed more than once", name);
					return out;
				}

				seen_names.push_back(name);

				if (eq == std::string::npos) {
					ids.push_back(name);
					names.push_back(name);
					continue;
				}

				const auto written_id = ::trimmed(entry.substr(eq + 1));
				const auto id = ::normalize_account_id(written_id);

				if (!::is_valid_account_id(written_id)) {
					error = typesafe_sprintf("--tournament-teams: %x: \"%x\" is not an account id (0x... or steam_...)", name, written_id);
					return out;
				}

				if (std::find(seen_ids.begin(), seen_ids.end(), id) != seen_ids.end()) {
					error = typesafe_sprintf("--tournament-teams: %x has the account id of another player", name);
					return out;
				}

				seen_ids.push_back(id);
				ids.push_back(id);
				names.push_back(name);
				++with_ids;
			}

			out.teams.push_back(ids);
			out.team_nicknames.push_back(names);
		}

		if (with_ids > 0 && with_ids != players) {
			std::string missing;

			for (std::size_t t = 0; t < out.teams.size(); ++t) {
				for (std::size_t p = 0; p < out.teams[t].size(); ++p) {
					if (out.teams[t][p] == out.team_nicknames[t][p]) {
						missing += (missing.empty() ? "" : ", ") + out.team_nicknames[t][p];
					}
				}
			}

			error = typesafe_sprintf("--tournament-teams: no account id for %x - give every player one (nickname=id), or none", missing);
			return out;
		}

		/*
			Nicknames only: the list is the roster itself.
		*/

		if (with_ids == 0) {
			out.team_nicknames.clear();
		}

		if (out.teams.size() < 2) {
			error = "--tournament-teams: expected at least 2 teams, one per line or separated by ;";
			return out;
		}

		/*
			Every team as big as --tournament-team-size, or without it as the first one.
		*/

		const auto expected_size = params.tournament_team_size.empty() ? out.teams[0].size() : std::size_t(out.open_team_size);

		for (const auto& team : out.teams) {
			if (team.size() != expected_size) {
				error = typesafe_sprintf("--tournament-teams: a team of %x players, expected %x", team.size(), expected_size);
				return out;
			}
		}
	}
	else if (const auto players = ::as_count(params.tournament_players, 2, 128)) {
		out.open_players = *players;

		if (out.open_players < 2 * out.open_team_size) {
			error = typesafe_sprintf("--tournament: %x players can't make two teams of %x", out.open_players, out.open_team_size);
			return out;
		}
	}
	else {
		error = typesafe_sprintf("--tournament: expected a number of players from 2 to 128, got \"%x\"", params.tournament_players);
		return out;
	}

	/*
		--tournament-join-timeout <minutes> | off - 10 minutes by default.
	*/

	if (params.tournament_join_timeout == "off") {
		out.join_timeout_secs = 0;
	}
	else if (!params.tournament_join_timeout.empty()) {
		if (const auto minutes = ::as_count(params.tournament_join_timeout, 1, 24 * 60)) {
			out.join_timeout_secs = *minutes * 60;
		}
		else {
			error = typesafe_sprintf("--tournament-join-timeout: expected off or minutes from 1 to 1440, got \"%x\"", params.tournament_join_timeout);
			return out;
		}
	}

	out.name = params.tournament_name;

	if (out.name.size() > 100) {
		error = "--tournament-name: at most 100 characters";
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

/*
	The live page's view of the tournament: every round of the bracket - the played ones from the history,
	the current one with its servers' live scores, the coming ones as empty places.
*/

namespace {
	using live_states = std::unordered_map<uint16_t, server_setup::live_match_state>;

	std::string round_name(const uint32_t round, const uint32_t rounds) {
		const auto from_end = rounds - 1 - round;

		if (from_end == 0) {
			return "Final";
		}

		if (from_end == 1) {
			return "Semifinals";
		}

		if (from_end == 2) {
			return "Quarterfinals";
		}

		return typesafe_sprintf("Round %x", round + 1);
	}

	tournament_page_state make_page_state(
		const tournament_config& cfg,
		const tournament_state& state,
		const live_states& live,
		const bool finished
	) {
		/*
			Everyone by their nickname and account - the nickname as listed, or as they played; an account where
			the nicknames are the identities is unknown (empty).
		*/

		std::unordered_map<std::string, std::string> nickname_of;

		for (const auto& team : state.teams) {
			for (std::size_t i = 0; i < team.player_ids.size() && i < team.nicknames.size(); ++i) {
				if (!team.nicknames[i].empty()) {
					nickname_of[team.player_ids[i]] = team.nicknames[i];
				}
			}
		}

		auto player = [&](const std::string& id) {
			auto p = tournament_page_player();
			const auto found = nickname_of.find(id);
			p.nickname = found != nickname_of.end() ? found->second : id;
			p.account = ::is_valid_account_id(id) ? id : std::string();
			return p;
		};

		auto team = [&](const std::vector<account_id_string>& ids) {
			tournament_page_team out;

			for (const auto& id : ids) {
				out.push_back(player(id));
			}

			return out;
		};

		auto live_team = [](const std::vector<std::string>& nicknames, const std::vector<std::string>& accounts) {
			tournament_page_team out;

			for (std::size_t i = 0; i < nicknames.size(); ++i) {
				auto p = tournament_page_player();
				p.nickname = nicknames[i];
				p.account = i < accounts.size() && ::is_valid_account_id(accounts[i]) ? accounts[i] : std::string();
				out.push_back(std::move(p));
			}

			return out;
		};

		tournament_page_state page;

		if (!cfg.name.empty()) {
			page.name = cfg.name;
		}

		page.state = finished ? "FINISHED" : "ONGOING";

		if (finished) {
			if (const auto winner = state.sole_surviving_team_index()) {
				page.winner_team = team(state.teams[*winner].player_ids);
			}
		}

		page.config.who_gets_in = cfg.is_open() ? "anyone" : cfg.team_nicknames.empty() ? "listed nicknames" : "listed accounts";
		page.config.team_size = cfg.is_open() ? cfg.open_team_size : static_cast<uint32_t>(cfg.teams.empty() ? 1 : cfg.teams.front().size());
		page.config.teams = cfg.num_teams();
		page.config.arenas = cfg.arenas;
		page.config.join_timeout_minutes = cfg.join_timeout_secs / 60;

		/*
			The rounds as they are now: before an open round 1 is over, the bracket made for the players declared;
			after it, the rounds left for the teams that are left - fewer came, fewer rounds.
		*/

		const auto rounds = [&]() {
			/*
				Over: the coordinator moved past the final already - stage_index is how many rounds were played.
			*/

			if (finished) {
				return std::max(1u, state.stage_index);
			}

			if (cfg.is_open() && state.stage_index == 0) {
				return tournament_config::rounds_for_teams(cfg.num_teams());
			}

			return state.stage_index + tournament_config::rounds_for_teams(static_cast<uint32_t>(state.surviving_team_indices().size()));
		}();

		for (uint32_t r = 0; r < rounds; ++r) {
			tournament_page_round round;
			round.name = round_name(r, rounds);
			round.arena = cfg.arenas.empty() ? std::string() : cfg.arena_for_rounds_left(rounds - r);

			const bool played = r < state.stage_index;
			const bool playing = r == state.stage_index && !finished;

			round.state = played ? "PLAYED" : playing ? "PLAYING" : "COMING";

			if (played) {
				for (const auto& h : state.match_history) {
					if (h.played_in_stage != r) {
						continue;
					}

					tournament_page_match m;
					m.team_a = team(h.winner_player_ids);
					m.team_b = team(h.loser_player_ids);
					m.winner = "a";

					if (h.walkover) {
						m.status = "walkover";
					}
					else {
						m.status = "done";
						m.score_a = static_cast<int>(h.winner_score);
						m.score_b = static_cast<int>(h.loser_score);
					}

					round.matches.push_back(std::move(m));
				}

				/*
					Its byes, from the history alone: a team that plays later on (or is still in it) but had no match here
					went on without playing.
				*/

				auto played_in = [&](const tournament_team_state& t, const auto in_stage) {
					return std::any_of(state.match_history.begin(), state.match_history.end(), [&](const auto& h) {
						return in_stage(h.played_in_stage) && (found_in(h.winner_player_ids, t.player_ids.front()) || found_in(h.loser_player_ids, t.player_ids.front()));
					});
				};

				for (const auto& t : state.teams) {
					if (t.player_ids.empty()) {
						continue;
					}

					const bool here = played_in(t, [r](const auto stage) { return stage == r; });
					const bool later = (!finished && !t.eliminated) || played_in(t, [r](const auto stage) { return stage > r; });

					if (later && !here) {
						round.byes.push_back(team(t.player_ids));
					}
				}
			}
			else if (playing) {
				std::vector<bool> plays(state.teams.size(), false);

				for (const auto& cm : state.current_stage_matches) {
					tournament_page_match m;

					if (cm.team_a_index < state.teams.size()) {
						plays[cm.team_a_index] = true;
						m.team_a = team(state.teams[cm.team_a_index].player_ids);
					}

					if (cm.team_b_index < state.teams.size()) {
						plays[cm.team_b_index] = true;
						m.team_b = team(state.teams[cm.team_b_index].player_ids);
					}

					if (cm.closed_empty) {
						m.status = "empty";
					}
					else if (cm.result.has_value()) {
						m.status = cm.result->walkover ? "walkover" : "done";
						m.winner = cm.result->winner_team_index == cm.team_a_index ? "a" : "b";

						if (!cm.result->walkover) {
							m.score_a = static_cast<int>(cm.result->team_a_score);
							m.score_b = static_cast<int>(cm.result->team_b_score);
						}
					}
					else {
						/*
							Not decided yet: where it's played, and its score once it's on.
						*/

						m.server_name = cm.server_name;
						m.port = cm.port;

						const auto found = live.find(cm.port);
						const bool started = found != live.end() && found->second.started;

						m.status = started ? "live" : "waiting";

						if (found != live.end()) {
							const auto& l = found->second;

							/*
								An open match has no teams yet - whoever took the places is on its server.
							*/

							if (cm.open_team_size > 0 && cm.team_a_index == no_tournament_team_v) {
								m.team_a = live_team(l.resistance, l.resistance_accounts);
								m.team_b = live_team(l.metropolis, l.metropolis_accounts);
							}

							if (started) {
								m.score_a = l.score_resistance;
								m.score_b = l.score_metropolis;
							}
						}
					}

					round.matches.push_back(std::move(m));
				}

				for (uint32_t t = 0; t < state.teams.size(); ++t) {
					if (!state.teams[t].eliminated && !plays[t]) {
						round.byes.push_back(team(state.teams[t].player_ids));
					}
				}
			}
			else {
				const auto places = 1u << (rounds - r - 1);

				for (uint32_t i = 0; i < places; ++i) {
					auto m = tournament_page_match();
					m.status = "coming";
					round.matches.push_back(std::move(m));
				}
			}

			page.rounds.push_back(std::move(round));
		}

		/*
			The final first - the newest on top, as the page shows them.
		*/

		std::reverse(page.rounds.begin(), page.rounds.end());
		return page;
	}
}

uint32_t tournament_config::rounds_for_teams(const uint32_t teams) {
	uint32_t rounds = 1;

	while ((1u << rounds) < teams) {
		++rounds;
	}

	return rounds;
}

const std::string& tournament_config::arena_for_rounds_left(const uint32_t rounds_left) const {
	const auto n = arenas.size();
	const auto index = rounds_left >= n ? std::size_t(0) : n - rounds_left;

	return arenas[index];
}

std::string tournament_config::compute_hash() const {
	/*
		The name is part of it: the same settings under a new name are a new tournament.
	*/

	const auto serialized = augs::to_json_string_nopretty(*this) + (name.empty() ? std::string() : "\nname: " + name);
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

	/*
		The live page: http://<this server>:<its first port>/ - the game's port number, but TCP.
		The state comes from the coordinator each time it's saved, the scores from the match servers once a second.
	*/

	const auto page_cfg = cfg;

	std::mutex page_lk;
	tournament_state page_state;
	live_states page_live;
	std::atomic<bool> page_finished = false;

	const auto page_signer = tournament_page_signer();
	auto avatars = tournament_avatars(in.state_file_path.parent_path() / "tournament_avatars");
	tournament_page_cache page_cache;
	tournament_page_rate_limit page_rate_limit;

	auto make_page_json = [&]() {
		/*
			Copied under the lock, serialized outside it - the match servers take the same lock every second.
		*/

		auto state_copy = tournament_state();
		auto live_copy = live_states();

		{
			std::scoped_lock lk(page_lk);
			state_copy = page_state;
			live_copy = page_live;
		}

		return augs::to_json_string_nopretty(::make_page_state(page_cfg, state_copy, live_copy, page_finished.load()));
	};

	/*
		--tournament-page: off, or the page's TCP port - by default the first game port's number.
	*/

	const auto& page_flag = in.params.tournament_page;
	const auto chosen_page_port = page_flag.empty() || page_flag == "off" ? std::optional<uint32_t>() : ::as_count(page_flag, 1, 65535);

	if (!page_flag.empty() && page_flag != "off" && !chosen_page_port.has_value()) {
		LOG("ERROR: --tournament-page: expected off or a port from 1 to 65535, got \"%x\"", page_flag);
		return work_result::FAILURE;
	}

	const auto page_port =
		page_flag == "off" ? uint16_t(0) :
		chosen_page_port.has_value() ? static_cast<uint16_t>(*chosen_page_port) :
		static_cast<uint16_t>(base_port)
	;

	std::unique_ptr<httplib::Server> page_server;
	std::thread page_thread;

	/*
		The port the matches tell the server list - none if the page couldn't take its port,
		so the list never links to a page that isn't there.
	*/

	auto served_page_port = uint16_t(0);

	if (page_port != 0) {
		const auto page_html = [&]() {
			try {
				return augs::file_to_string(DETAIL_DIR / "web" / "tournament_page.html");
			}
			catch (const std::exception& err) {
				LOG("Tournament: no live page - could not read it: %x", err.what());
				return std::string();
			}
		}();

		page_server = std::make_unique<httplib::Server>();

		page_server->Get("/", [page_html](const httplib::Request&, httplib::Response& res) {
			res.set_content(page_html, "text/html; charset=utf-8");
		});

		page_server->Get(R"(/miniature/([A-Za-z0-9_\-]+)\.png)", [miniatures = std::make_shared<tournament_arena_miniatures>(page_cfg.arenas)](const httplib::Request& req, httplib::Response& res) {
			if (const auto png = miniatures->find(req.matches[1].str())) {
				res.set_header("Cache-Control", "max-age=3600");
				res.set_content(*png, "image/png");
			}
			else {
				res.status = 404;
			}
		});

		page_server->Get("/avatar", [&avatars](const httplib::Request& req, httplib::Response& res) {
			/*
				Also "none" kept a minute by the browser - not asked again with every redraw of the page.
			*/

			res.set_header("Cache-Control", "max-age=60");

			if (const auto png = avatars.find(req.get_param_value("account"))) {
				res.set_content(*png, "image/png");
			}
			else {
				res.status = 404;
			}
		});

		page_server->Get(R"(/([a-z_0-9@]+\.png))", [pictures = std::make_shared<tournament_page_pictures>()](const httplib::Request& req, httplib::Response& res) {
			if (const auto png = pictures->find(req.matches[1].str())) {
				res.set_header("Cache-Control", "max-age=3600");
				res.set_content(*png, "image/png");
			}
			else {
				res.status = 404;
			}
		});

		/*
			Only the JSON is limited per address - making it is what costs; the pictures are served as they are.
		*/

		page_server->Get("/tournament.json", [&page_cache, &page_signer, &make_page_json, &page_rate_limit](const httplib::Request& req, httplib::Response& res) {
			if (!page_rate_limit.allow(req.remote_addr)) {
				res.status = 429;
				res.set_header("Retry-After", "1");
				return;
			}

			const auto page = page_cache.get(make_page_json, page_signer);

			for (const auto& header : page.headers) {
				res.set_header(header.first, header.second);
			}

			res.set_header("Cache-Control", "no-store");
			res.set_header("Access-Control-Allow-Origin", "*");
			res.set_header("Access-Control-Expose-Headers", "X-Hypersomnia-Server-Key, X-Hypersomnia-Signature");
			res.set_content(page.json, "application/json");
		});

		/*
			Bound here, before the matches start - they tell the server list the page's port only if it's taken.
			Running before going on, so stop() at the end always finds it listening.
		*/

		if (page_server->bind_to_port("0.0.0.0", page_port)) {
			page_thread = std::thread([&page_server]() {
				page_server->listen_after_bind();
			});

			page_server->wait_until_ready();
			served_page_port = page_port;

			LOG("Tournament: the live page is at http://<this server>:%x/ (TCP %x).", page_port, page_port);
		}
		else {
			LOG("Tournament: the live page could not take TCP port %x - not served.", page_port);
			page_server.reset();
		}
	}

	tournament_coordinator_dependencies deps;

	deps.on_state_changed = [&page_lk, &page_state](const tournament_state& s) {
		std::scoped_lock lk(page_lk);
		page_state = s;
	};
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

	/*
		Whether a player has no match of their own to play now - asked by the match servers (may_spectate_tournament);
		the coordinator answers once it exists.
	*/

	std::function<bool(const std::function<bool(const std::string&)>&)> may_watch_others;
	std::function<void(tournament_match&, const account_id_string&, const account_id_string&)> claim_listed_nickname;

	deps.run_stage = [&](
		std::vector<tournament_match>& matches,
		tournament_commit_match_result_fn commit
	) {
		std::atomic<bool> any_failure = false;

		/*
			A port used again in this stage: the last stage's final score there isn't this match's live one.
		*/

		{
			std::scoped_lock lk(page_lk);

			for (const auto& m : matches) {
				page_live.erase(m.port);
			}
		}

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

		/*
			The join timeout of every match of this round counts from the round's first player coming.
		*/

		const auto round_first_arrival = std::make_shared<std::atomic<double>>(-1.0);

		/*
			Started again after a crash: the players of the matches decided before it came already - counted,
			so round 1 still closes by itself once the rest are here.
		*/

		const auto already_came = [&]() {
			uint32_t n = 0;

			for (const auto& m : matches) {
				if (m.open_team_size > 0 && m.result.has_value()) {
					n += static_cast<uint32_t>(m.team_a_player_ids.size() + m.team_b_player_ids.size());
				}
			}

			return n;
		}();

		const auto open_round_players = std::make_shared<std::atomic<uint32_t>>(already_came);

		/*
			Who has a place on which open match - one place per account; the decided matches' players have theirs.
		*/

		const auto open_places = std::make_shared<open_round_places>();

		for (const auto& m : matches) {
			if (m.open_team_size > 0 && m.result.has_value()) {
				for (const auto& ids : { m.team_a_player_ids, m.team_b_player_ids }) {
					for (const auto& id : ids) {
						open_places->take(id, m.server_name);
					}
				}
			}
		}

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
				The players are always told apart by their accounts. A roster listed by nicknames only holds nick_<nickname>
				for whoever hasn't come yet - the first account to come with that nickname takes it (claim_listed_nickname).
			*/

			if (this_config.server.authenticate_with_nicknames) {
				LOG("Tournament match on port %x: authenticate_with_nicknames turned off - the players are their accounts.", match.port);
				this_config.server.authenticate_with_nicknames = false;
			}

			auto fill_faction = [&roster](const std::vector<account_id_string>& ids, const faction_type f) {
				for (const auto& id : ids) {
					roster.id_to_faction[id] = f;
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
				stage_should_interrupt,
				on_instance_exit,
				commit,
				close_open_round,
				round_first_arrival,
				open_round_players,
				open_places,
				expected_players = in.config.open_players,
				join_timeout_secs = in.config.join_timeout_secs,
				&may_watch_others,
				&claim_listed_nickname,
				&avatars,
				served_page_port,
				&page_lk,
				&page_live,
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

				server_ptr->tournament_page_port = served_page_port;

				/*
					The join timeout: a match of known teams decided by who came (server_join_timeout.hpp) -
					an open one closes as Close round 1 does.
				*/

				server_ptr->join_timeout_secs = join_timeout_secs;
				server_ptr->round_first_arrival = round_first_arrival;

				/*
					A team with no match of its own to play now watches any match.
				*/

				server_ptr->may_spectate_tournament = [&may_watch_others](const std::string& account) {
					if (!may_watch_others) {
						return false;
					}

					return may_watch_others([&](const std::string& player_id) {
						return ::normalize_account_id(player_id) == account;
					});
				};

				server_ptr->on_listed_nickname_claimed = [match_ptr, &claim_listed_nickname](const std::string& placeholder, const std::string& account) {
					claim_listed_nickname(*match_ptr, placeholder, account);
				};
				server_ptr->on_join_timeout = [match_ptr, commit](const faction_type came) {
					auto outcome = tournament_match_outcome();
					outcome.walkover = true;
					outcome.nobody_came = came == faction_type::COUNT;
					outcome.team_a_won = came == team_a_faction;
					commit(*match_ptr, outcome);
				};
				server_ptr->on_player_avatar = [&avatars](const std::string& player, const std::vector<std::byte>& png) {
					avatars.save(player, png);
				};

				server_ptr->on_live_state = [&page_lk, &page_live, port = match_ptr->port](const server_setup::live_match_state& s) {
					std::scoped_lock lk(page_lk);
					page_live[port] = s;
				};

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
					server_ptr->open_places = open_places;
					server_ptr->open_match_server_name = match_ptr->server_name;
					server_ptr->open_round_expected_players = expected_players;

					/*
						Closed before its pair was there: whoever waited advances without playing - as one team of at most
						the team size, whichever side they took: the fuller side, filled up from the other one.
						Who doesn't fit is out of this match, its place freed. Nobody: an empty server, gone.
					*/

					server_ptr->on_open_match_closed = [match_ptr, commit, take_open_teams, open_places](const server_assigned_teams& roster) {
						auto outcome = tournament_match_outcome();
						outcome.walkover = true;

						take_open_teams(roster, outcome);

						if (outcome.open_team_b.size() > outcome.open_team_a.size()) {
							std::swap(outcome.open_team_a, outcome.open_team_b);
							std::swap(outcome.open_names_a, outcome.open_names_b);
						}

						const auto team_size = static_cast<std::size_t>(match_ptr->open_team_size);

						for (std::size_t i = 0; i < outcome.open_team_b.size(); ++i) {
							if (outcome.open_team_a.size() < team_size) {
								outcome.open_team_a.push_back(outcome.open_team_b[i]);
								outcome.open_names_a.push_back(outcome.open_names_b[i]);
							}
							else {
								/*
									Its place freed: it played nothing - another open match still waiting may take it.
								*/

								LOG("Tournament: %x (%x) didn't fit into a team of %x - out.", outcome.open_names_b[i], outcome.open_team_b[i], team_size);

								if (open_places != nullptr) {
									open_places->free(outcome.open_team_b[i], match_ptr->server_name);
								}
							}
						}

						outcome.open_team_b.clear();
						outcome.open_names_b.clear();

						commit(*match_ptr, outcome);
					};
				}

				server_ptr->on_match_summary = [match_ptr, commit, server_raw, take_open_teams](const messages::match_summary_message& s) {
					auto outcome = tournament_match_outcome();

					if (match_ptr->open_team_size > 0 && match_ptr->team_a_index == no_tournament_team_v) {
						take_open_teams(server_raw->get_assigned_teams(), outcome);
					}

					const auto& team_a_ids = match_ptr->open_team_size > 0 ? outcome.open_team_a : match_ptr->team_a_player_ids;
					const auto& team_b_ids = match_ptr->open_team_size > 0 ? outcome.open_team_b : match_ptr->team_b_player_ids;

					// a listed team's nicknames, as they played - in its order
					if (match_ptr->open_team_size == 0) {
						for (const auto& id : team_a_ids) {
							outcome.open_names_a.push_back(server_raw->find_nickname_of_account(id));
						}

						for (const auto& id : team_b_ids) {
							outcome.open_names_b.push_back(server_raw->find_nickname_of_account(id));
						}
					}

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
						return found_in(roster, id);
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

	/*
		Over: stay up rather than quit - a container would only be started again and again (restart: unless-stopped) -
		with the live page showing the results.
	*/

	auto idle_until_interrupted = [&in]() {
		while (!(in.handle_sigint && in.handle_sigint())) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	};

	auto coordinator = tournament_coordinator(
		std::move(cfg),
		in.state_file_path,
		std::move(deps)
	);

	may_watch_others = [&coordinator](const std::function<bool(const std::string&)>& is_player) {
		return coordinator.may_watch_others(is_player);
	};

	claim_listed_nickname = [&coordinator](tournament_match& m, const account_id_string& placeholder, const account_id_string& account) {
		coordinator.claim_listed_nickname(m, placeholder, account);
	};

	/*
		Its results saved as tournament_results.json and POSTed to the webhook - when it's over, and again whenever
		a tournament over already is started (after a stop right at its end, a webhook that didn't answer):
		the receiver drops a second copy.
	*/

	auto publish_results = [&]() {
		page_finished = true;

		const auto results_json = make_page_json();
		::save_tournament_results(in.state_file_path.parent_path() / "tournament_results.json", results_json);
		::post_tournament_results(in.config_pattern.server_private.tournament_results_webhook_url, results_json, page_signer, in.handle_sigint);
	};

	if (!coordinator.run()) {
		if (const auto played = tournament_state::load(coordinator.completed_archive)) {
			{
				std::scoped_lock lk(page_lk);
				page_state = *played;
			}

			publish_results();
		}

		page_finished = true;
		idle_until_interrupted();
	}
	else if (coordinator.aborted) {
		/*
			Not over - a match ended without a result. Quit with a failure rather than show it finished.
		*/

		LOG("ERROR: Tournament: stopped - a match ended without a result. Start it again to carry on.");

		if (page_server) {
			page_server->stop();
		}

		if (page_thread.joinable()) {
			page_thread.join();
		}

		return work_result::FAILURE;
	}
	else if (!(in.handle_sigint && in.handle_sigint())) {
		publish_results();
		LOG("Tournament: over - the live page shows the results. Ctrl+C (or stopping the service) to quit.");
		idle_until_interrupted();
	}

	if (page_server) {
		page_server->stop();
	}

	if (page_thread.joinable()) {
		page_thread.join();
	}

	return work_result::SUCCESS;
}
#endif
