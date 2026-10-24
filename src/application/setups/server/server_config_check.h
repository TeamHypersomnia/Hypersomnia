#pragma once
#include <string>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/string/string_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "all_paths.h"
#include "application/config_json_table.h"
#include "application/setups/server/admin_profiles.h"
#include "application/setups/server/server_slots.h"

/*
	A dedicated server's config checked at start - what loading it passes over without a word:
	a typo in a key, text where a number goes, a game mode or an enum value that doesn't exist, an admin that can't be.
	A tournament doesn't start with any of them - nothing may surprise it once it's on.
	Other servers only warn, so a config from an older version still starts.
*/

struct server_config_source {
	augs::path_type path;

	/*
		The text itself if it has no file (HYPERSOMNIA_CONFIG) - otherwise read from path.
	*/

	std::string content;

	/*
		Its problems stop the start - a tournament's config, not runtime_prefs.json (the game's own).
	*/

	bool strict = false;
};

inline std::string read_config_source(const server_config_source& source) {
	if (!source.content.empty()) {
		return source.content;
	}

	try {
		return augs::file_to_string(source.path);
	}
	catch (...) {
		return std::string();
	}
}

/*
	False: a tournament mustn't start - the reasons are in the log.
*/

inline bool check_server_config(
	const std::vector<server_config_source>& sources,
	const augs::path_type& default_config_path,
	const config_json_table& config,
	const bool tournament
) {
	auto any_error = false;

	const auto default_config_json = ::read_config_source({ default_config_path, "", false });

	for (const auto& source : sources) {
		for (const auto& problem : config_json_table::find_problems(::read_config_source(source), default_config_json)) {
			if (source.strict) {
				LOG("ERROR: %x: %x", source.path, problem);
				any_error = true;
			}
			else {
				LOG("WARNING! %x: %x - ignored.", source.path, problem);
			}
		}
	}

	/*
		As the config ends up: an admin's rcon level or account id that can't be is an admin that never is;
		a listed player's id that can't be, a player never let in; slots past what the netcode has room for.
	*/

	auto settled_problems = std::vector<std::string>();

	for (const auto& account : config.server_private.allowed_accounts) {
		if (!::is_valid_account_id(account)) {
			settled_problems.push_back(typesafe_sprintf("allowed_accounts: \"%x\" is not an account id (steam_..., 0x... or key_...)", account));
		}
	}

	if (config.server_start.slots > max_server_slots_v) {
		settled_problems.push_back(typesafe_sprintf("server_start.slots: %x - at most %x (the rest are kept for admins and returning players)", config.server_start.slots, max_server_slots_v));
	}

	for (const auto& problem : settled_problems) {
		if (tournament) {
			LOG("ERROR: %x", problem);
			any_error = true;
		}
		else {
			LOG("WARNING! %x - %x is used.", problem, problem.rfind("server_start.slots", 0) == 0 ? "the most there can be" : "it's ignored");
		}
	}

	for (const auto& entry : config.server_private.admin_profiles) {
		auto problems = std::vector<std::string>();

		if (!::is_valid_account_id(entry.account)) {
			problems.push_back(typesafe_sprintf("admin_profiles: \"%x\" is not an account id (steam_..., 0x... or key_...)", entry.account));
		}

		if (!::is_valid_rcon_level_name(entry.rcon)) {
			problems.push_back(typesafe_sprintf("admin_profiles: \"%x\" for %x is not one of: NONE, RCON, MASTER_RCON", entry.rcon, entry.account));
		}

		for (const auto& problem : problems) {
			if (tournament) {
				LOG("ERROR: %x", problem);
				any_error = true;
			}
			else {
				LOG("WARNING! %x - this admin is ignored.", problem);
			}
		}
	}

	if (any_error) {
		LOG("ERROR: A tournament doesn't start with a config it can't read as written - fix the lines above.");
	}

	return !any_error;
}

/*
	The game modes an arena has - the keys of "game_modes" in its <arena>.json.
*/

inline std::vector<std::string> game_modes_of_arena(const augs::path_type& arena_dir, const std::string& arena) {
	std::vector<std::string> names;

	try {
		constexpr auto flags = rapidjson::kParseDefaultFlags | rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag;
		const auto doc = augs::json_document_from<flags>(augs::file_to_string(arena_dir / (arena + ".json")));

		if (doc.HasMember("game_modes") && doc["game_modes"].IsObject()) {
			for (const auto& it : doc["game_modes"].GetObject()) {
				names.push_back(it.name.GetString());
			}
		}
	}
	catch (...) {
	}

	return names;
}

/*
	A tournament's arenas, after the download: each one on disk, with the game mode it's given -
	a typo stops the start instead of a match on another arena, or none, in the middle of the tournament.
	False: it mustn't start - the reasons are in the log.
*/

inline bool check_tournament_arenas(const std::string& tournament_arenas) {
	auto any_error = false;

	auto entries = std::vector<std::string>();
	auto current = std::string();

	for (const auto c : tournament_arenas + ",") {
		if (c != ',') {
			current += c;
			continue;
		}

		if (const auto first = current.find_first_not_of(" \t\r\n"); first != std::string::npos) {
			entries.push_back(current.substr(first));
		}

		current.clear();
	}

	for (const auto& entry : entries) {
		const auto arena = ::get_first_word(entry);
		const auto mode = ::get_second_word(entry);

		const auto dir = [&]() {
			if (augs::exists(OFFICIAL_ARENAS_DIR / arena)) {
				return OFFICIAL_ARENAS_DIR / arena;
			}

			if (augs::exists(DOWNLOADED_ARENAS_DIR / arena)) {
				return DOWNLOADED_ARENAS_DIR / arena;
			}

			return augs::path_type();
		}();

		if (dir.empty()) {
			LOG("ERROR: --tournament-arenas: no arena \"%x\" - neither in the game nor among the community arenas (hypersomnia.io/arenas).", arena);
			any_error = true;
			continue;
		}

		if (mode.empty()) {
			continue;
		}

		const auto modes = ::game_modes_of_arena(dir, arena);

		if (!found_in(modes, mode)) {
			std::string listed;

			for (const auto& m : modes) {
				listed += (listed.empty() ? "" : ", ") + m;
			}

			LOG("ERROR: --tournament-arenas: %x has no game mode \"%x\" - it has: %x", arena, mode, listed);
			any_error = true;
		}
	}

	return !any_error;
}
