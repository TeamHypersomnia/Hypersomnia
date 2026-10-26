#pragma once
#include <string>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/string/string_templates.h"
#include "augs/string/parse_url.h"
#include "augs/templates/algorithm_templates.h"
#include "all_paths.h"
#include "application/config_json_table.h"
#include "application/setups/server/admin_profiles.h"
#include "application/setups/server/server_slots.h"
#include "application/setups/editor/project/editor_project_paths.h"

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

	/*
		Where the nicknames are the identities, nobody proves an account - a list of accounts would let in anyone
		under a padlock. Never started so, also outside a tournament.
	*/

	if (!config.server_private.allowed_accounts.empty() && config.server.authenticate_with_nicknames) {
		LOG("ERROR: allowed_accounts with authenticate_with_nicknames: the players are known only by their nicknames, so the list can't be checked - remove one of them.");
		any_error = true;
	}

	if (const auto& spectator = config.server_private.spectator_account; !spectator.empty() && !::is_valid_account_id(spectator)) {
		settled_problems.push_back(typesafe_sprintf("spectator_account: \"%x\" is not an account id (steam_..., 0x... or key_...)", spectator));
	}

	if (const auto& url = config.server_private.tournament_results_webhook_url; !url.empty() && !(parsed_url(url).valid() && (url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0))) {
		settled_problems.push_back("tournament_results_webhook_url is not a URL (https://...)");
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
	A tournament's maps, after the download: each one on disk - a typo stops the start instead of a match on another
	map, or none, in the middle of the tournament.
	False: it mustn't start - the reasons are in the log.
*/

inline bool check_tournament_maps(const std::string& tournament_maps) {
	auto any_error = false;

	auto entries = std::vector<std::string>();
	auto current = std::string();

	/*
		Apart by commas or lines, as the tournament reads them.
	*/

	for (const auto c : tournament_maps + ",") {
		/*
			A list written on Windows: its lines end with \r too.
		*/

		if (c == '\r') {
			continue;
		}

		if (c != ',' && c != '\n') {
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

		/*
			Found as the match servers will find it - official, a project, downloaded or custom/maps; "..", never.
		*/

		if (::server_arena_folder_of(arena, true).empty()) {
			LOG("ERROR: --tournament-maps: no map \"%x\" - neither in the game, nor among the community maps (hypersomnia.io/arenas), nor in custom/maps.", arena);
			any_error = true;
			continue;
		}

		/*
			No check of the mode: a tournament plays bomb defusal, and every map has every mode - those it doesn't
			list in its JSON are the defaults (create_all_missing_modes).
		*/
	}

	return !any_error;
}
