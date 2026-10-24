#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <regex>
#include "augs/string/typesafe_sprintf.h"
#include "game/modes/difficulty_type.h"
#include "application/network/account_id.h"
#include "application/config_json_table.h"
#include "application/main/environment_variables.h"
#include "application/setups/server/server_slots.h"

#if PLATFORM_WINDOWS
#include <stdlib.h>
#define PROCESS_ENVIRONMENT _environ
#else
extern char** environ;
#define PROCESS_ENVIRONMENT environ
#endif

namespace {
	/*
		A value it can't read throws this - the message names what was expected.
	*/

	struct bad_value {
		std::string expected;
	};

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

		for (const auto c : value) {
			if (separators.find(c) != std::string::npos) {
				if (const auto item = ::trimmed(current); !item.empty()) {
					out.push_back(item);
				}

				current.clear();
			}
			else {
				current += c;
			}
		}

		if (const auto item = ::trimmed(current); !item.empty()) {
			out.push_back(item);
		}

		return out;
	}

	int as_number(const std::string& value, const int min, const int max) {
		static const auto number = std::regex("[0-9]{1,9}");

		if (!std::regex_match(value, number)) {
			throw bad_value { typesafe_sprintf("a number from %x to %x", min, max) };
		}

		const auto n = std::atoi(value.c_str());

		if (n < min || n > max) {
			throw bad_value { typesafe_sprintf("a number from %x to %x", min, max) };
		}

		return n;
	}

	/*
		The bots' difficulty as in the config: VERY_EASY, EASY, MEDIUM, HARD, PROGRESSIVE (any case).
	*/

	difficulty_type as_bot_difficulty(const std::string& value) {
		auto upper = value;
		std::transform(upper.begin(), upper.end(), upper.begin(), [](const unsigned char c) { return static_cast<char>(std::toupper(c)); });

		const std::pair<const char*, difficulty_type> names[] = {
			{ "VERY_EASY", difficulty_type::VERY_EASY },
			{ "EASY", difficulty_type::EASY },
			{ "MEDIUM", difficulty_type::MEDIUM },
			{ "HARD", difficulty_type::HARD },
			{ "PROGRESSIVE", difficulty_type::PROGRESSIVE }
		};

		for (const auto& [name, difficulty] : names) {
			if (upper == name) {
				return difficulty;
			}
		}

		throw bad_value { "one of VERY_EASY, EASY, MEDIUM, HARD, PROGRESSIVE" };
	}

	/*
		How many bots: 0 - none; the map's own number when the variable isn't set.
	*/

	void set_bot_quota(config_json_table& c, const std::string& value) {
		c.server.bot_quota.clear();
		c.server.bot_quota.push_back(static_cast<uint8_t>(::as_number(value, 0, 64)));
	}

	/*
		"off" turns a thing off; "on" is accepted too, for symmetry.
	*/

	bool as_on_off(const std::string& value) {
		if (value == "off") {
			return false;
		}

		if (value == "on") {
			return true;
		}

		throw bad_value { "off or on" };
	}

	/*
		"fy_iceworld bomb_defusal" - the arena, then its game mode if not the default one.
	*/

	void as_arena_and_mode(const std::string& value, std::string& arena, std::string& mode) {
		const auto words = ::split_list(value, " ");

		if (words.empty() || words.size() > 2) {
			throw bad_value { "an arena, optionally followed by a game mode, e.g. fy_iceworld bomb_defusal" };
		}

		arena = words[0];
		mode = words.size() > 1 ? words[1] : std::string();
	}

	void set_arena(config_json_table& c, const std::string& value) {
		std::string arena;
		std::string mode;

		::as_arena_and_mode(value, arena, mode);

		c.server.arena = arena;
		c.server.game_mode = mode;
	}

	/*
		The arenas that follow each other: "de_silo,fy_iceworld bomb_defusal".
	*/

	void set_map_cycle(config_json_table& c, const std::string& value) {
		const auto entries = ::split_list(value, ",");

		if (entries.empty()) {
			throw bad_value { "arenas separated by commas, e.g. de_cyberaqua,de_silo" };
		}

		c.server.cycle = arena_cycle_type::LIST;
		c.server.cycle_list.clear();

		for (const auto& entry : entries) {
			std::string arena;
			std::string mode;

			::as_arena_and_mode(entry, arena, mode);
			c.server.cycle_list.push_back(mode.empty() ? arena : arena + " " + mode);
		}
	}

	/*
		Account ids with full rights: "steam_7656119...,0x8aed41f2...".
	*/

	void set_master_admins(config_json_table& c, const std::string& value) {
		const auto ids = ::split_list(value, ", \t\n");

		if (ids.empty()) {
			throw bad_value { "account ids separated by commas, e.g. steam_76561198000000000,0x8aed41f2..." };
		}

		c.server_private.admin_profiles.clear();

		for (const auto& id : ids) {
			if (!::is_valid_account_id(id)) {
				throw bad_value { "account ids (steam_..., 0x... or key_...) separated by commas" };
			}

			auto entry = admin_profile_entry();
			entry.account = ::normalize_account_id(id);
			entry.rcon = "MASTER_RCON";

			c.server_private.admin_profiles.push_back(entry);
		}
	}

	/*
		The only accounts let in (admins always are): "steam_765...,0x8aed...".
	*/

	void set_allowed_accounts(config_json_table& c, const std::string& value) {
		const auto ids = ::split_list(value, ", \t\n");

		if (ids.empty()) {
			throw bad_value { "account ids separated by commas, e.g. steam_76561198000000000,0x8aed41f2..." };
		}

		c.server_private.allowed_accounts.clear();

		for (const auto& id : ids) {
			if (!::is_valid_account_id(id)) {
				throw bad_value { "account ids (steam_..., 0x... or key_...) separated by commas" };
			}

			c.server_private.allowed_accounts.push_back(::normalize_account_id(id));
		}
	}

	/*
		"off", or the hour of the daily update: "05:00", "19:30".
	*/

	void set_daily_update(config_json_table& c, const std::string& value) {
		if (value == "off") {
			c.server.daily_autoupdate = false;
			return;
		}

		static const auto hour_and_minute = std::regex("([01][0-9]|2[0-3]):[0-5][0-9]");

		if (!std::regex_match(value, hour_and_minute)) {
			throw bad_value { "off or an hour, e.g. 05:00" };
		}

		c.server.daily_autoupdate = true;
		c.server.daily_autoupdate_hour = value;
	}

	using env_setter = std::function<void(config_json_table&, const std::string&)>;

	struct environment_variable {
		const char* name;
		env_setter set;
	};

	/*
		One line per variable. The fields are the ones the config generator writes into server.json for the AppImage.
	*/

	const auto config_variables = std::array {
		environment_variable { "HYPERSOMNIA_SERVER_NAME", [](auto& c, const auto& v) { c.server.server_name = v; } },
		environment_variable { "HYPERSOMNIA_ARENA", [](auto& c, const auto& v) { ::set_arena(c, v); } },
		environment_variable { "HYPERSOMNIA_MAP_CYCLE", [](auto& c, const auto& v) { ::set_map_cycle(c, v); } },
		environment_variable { "HYPERSOMNIA_MAP_CYCLE_RANDOM", [](auto& c, const auto& v) { c.server.cycle_randomize_order = ::as_on_off(v); } },
		environment_variable { "HYPERSOMNIA_ADMINS", [](auto& c, const auto& v) { ::set_master_admins(c, v); } },
		environment_variable { "HYPERSOMNIA_ALLOWED_ACCOUNTS", [](auto& c, const auto& v) { ::set_allowed_accounts(c, v); } },
		environment_variable { "HYPERSOMNIA_PASSWORD", [](auto& c, const auto& v) { c.server_private.server_password = v; } },
		environment_variable { "HYPERSOMNIA_SLOTS", [](auto& c, const auto& v) { c.server_start.slots = ::as_number(v, 1, max_server_slots_v); } },
		environment_variable { "HYPERSOMNIA_SERVER_INSTANCES", [](auto& c, const auto& v) { c.instances.casual = ::as_number(v, 1, 20); } },
		environment_variable { "HYPERSOMNIA_PORT", [](auto& c, const auto& v) { c.server_start.port = ::as_number(v, 1, 65535); } },
		environment_variable { "HYPERSOMNIA_WEB_PORT", [](auto& c, const auto& v) { c.server.webrtc_port_range_begin = ::as_number(v, 1, 65535); } },
		environment_variable { "HYPERSOMNIA_BROWSER_PLAYERS", [](auto& c, const auto& v) { c.server.allow_webrtc_clients = ::as_on_off(v); } },
		environment_variable { "HYPERSOMNIA_BOT_QUOTA", [](auto& c, const auto& v) { ::set_bot_quota(c, v); } },
		environment_variable { "HYPERSOMNIA_BOT_DIFFICULTY", [](auto& c, const auto& v) { c.server.bot_difficulty = ::as_bot_difficulty(v); } },
		environment_variable { "HYPERSOMNIA_DAILY_UPDATE", [](auto& c, const auto& v) { ::set_daily_update(c, v); } },
		environment_variable { "HYPERSOMNIA_TIME_ZONE", [](auto& c, const auto& v) { c.server.time_zone = v; } },
		environment_variable { "HYPERSOMNIA_DISCORD_WEBHOOK", [](auto& c, const auto& v) { c.server_private.discord_webhook_url = v; } }
	};

	/*
		Read elsewhere, so known - no warning when they're set.
	*/

	const auto variables_read_elsewhere = std::array {
		"HYPERSOMNIA_CONFIG",
		"HYPERSOMNIA_SERVER_PRIVATE_KEY"
	};

	/*
		The edit distance - for "did you mean".
	*/

	std::size_t distance(const std::string& a, const std::string& b) {
		std::vector<std::size_t> row(b.size() + 1);

		for (std::size_t j = 0; j <= b.size(); ++j) {
			row[j] = j;
		}

		for (std::size_t i = 1; i <= a.size(); ++i) {
			auto diagonal = row[0];
			row[0] = i;

			for (std::size_t j = 1; j <= b.size(); ++j) {
				const auto above = row[j];
				row[j] = std::min({ row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1) });
				diagonal = above;
			}
		}

		return row[b.size()];
	}

	std::vector<std::string> known_variables() {
		std::vector<std::string> out;

		for (const auto& v : config_variables) {
			out.push_back(v.name);
		}

		for (const auto& f : environment_flags) {
			out.push_back(f.variable);
		}

		for (const auto& n : variables_read_elsewhere) {
			out.push_back(n);
		}

		return out;
	}
}

std::vector<std::string> apply_config_environment_variables(config_json_table& config) {
	std::vector<std::string> errors;

	for (const auto& v : config_variables) {
		const auto value = std::getenv(v.name);

		if (value == nullptr) {
			continue;
		}

		try {
			v.set(config, ::trimmed(value));
		}
		catch (const bad_value& err) {
			errors.push_back(typesafe_sprintf("%x: expected %x, got \"%x\"", v.name, err.expected, value));
		}
	}

	return errors;
}

std::vector<std::string> unknown_environment_variable_warnings() {
	const auto known = ::known_variables();
	const auto prefix = std::string("HYPERSOMNIA_");

	std::vector<std::string> warnings;

	for (auto entry = PROCESS_ENVIRONMENT; entry != nullptr && *entry != nullptr; ++entry) {
		const auto line = std::string(*entry);
		const auto name = line.substr(0, line.find('='));

		if (name.rfind(prefix, 0) != 0 || std::find(known.begin(), known.end(), name) != known.end()) {
			continue;
		}

		const auto closest = *std::min_element(known.begin(), known.end(), [&name](const auto& a, const auto& b) {
			return ::distance(name, a) < ::distance(name, b);
		});

		warnings.push_back(
			::distance(name, closest) <= 3 ?
			typesafe_sprintf("%x is not a known variable - did you mean %x?", name, closest) :
			typesafe_sprintf("%x is not a known variable - nothing reads it.", name)
		);
	}

	return warnings;
}
