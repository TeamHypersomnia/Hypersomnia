#pragma once
#include <array>
#include <string>
#include <vector>

struct config_json_table;

/*
	The HYPERSOMNIA_* environment variables - for Docker, where a container is configured by them.
	README_SERVER.md's table and the config generator (hypersomnia.io/generator) follow the lists in this file
	and environment_variables.cpp - .claude/skills/local-testing/scripts/check_environment_variables.sh checks
	that all three name the same ones.

	Two kinds:
	- The ones that set the config: applied over conf.d/ and HYPERSOMNIA_CONFIG, under runtime_prefs.json -
	  so a change made from the administration panel survives the container's daily restart.
	- The ones that are command line flags: the variable is the flag, e.g. HYPERSOMNIA_TOURNAMENT=16 is --tournament 16.
	  A flag given on the command line wins. A flag without a value is given when its variable is 1 (or on, true, yes).
*/

struct environment_flag {
	const char* variable;
	const char* flag;
	bool without_value = false;
};

inline constexpr auto environment_flags = std::array {
	environment_flag { "HYPERSOMNIA_TOURNAMENT", "--tournament" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_TEAM_SIZE", "--tournament-team-size" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_TEAMS", "--tournament-teams" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_MAPS", "--tournament-maps" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_PAGE", "--tournament-page" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_ROUND_1_MAX_WAIT", "--tournament-round-1-max-wait" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_NEXT_ROUNDS_MAX_WAIT", "--tournament-next-rounds-max-wait" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_SPECTATOR_SLOTS", "--tournament-spectator-slots" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_NAME", "--tournament-name" },
	environment_flag { "HYPERSOMNIA_DOWNLOAD_MAPS_AND_QUIT", "--download-maps-and-quit", true }
};

/*
	A tournament's setting under its old name (a variable or a flag): what it is now - empty if it isn't an old one.
*/

inline std::string renamed_tournament_setting(const std::string& old_name) {
	if (old_name == "HYPERSOMNIA_TOURNAMENT_ARENAS") {
		return "HYPERSOMNIA_TOURNAMENT_MAPS";
	}

	if (old_name == "HYPERSOMNIA_TOURNAMENT_SPECTATORS") {
		return "HYPERSOMNIA_TOURNAMENT_SPECTATOR_SLOTS";
	}

	if (old_name == "HYPERSOMNIA_TOURNAMENT_JOIN_TIMEOUT") {
		return "HYPERSOMNIA_TOURNAMENT_ROUND_1_MAX_WAIT and HYPERSOMNIA_TOURNAMENT_NEXT_ROUNDS_MAX_WAIT";
	}

	if (old_name == "--tournament-arenas") {
		return "--tournament-maps";
	}

	if (old_name == "--tournament-spectators") {
		return "--tournament-spectator-slots";
	}

	if (old_name == "--tournament-join-timeout") {
		return "--tournament-round-1-max-wait and --tournament-next-rounds-max-wait";
	}

	return "";
}

/*
	Sets the config from the variables that are set. Returns the errors - a value it can't read, e.g.
	"HYPERSOMNIA_SLOTS: expected a number, got abc" - the server shouldn't start with those.
*/

std::vector<std::string> apply_config_environment_variables(config_json_table&);

/*
	"... is not a known variable - did you mean HYPERSOMNIA_SLOTS?" for every set HYPERSOMNIA_*
	variable that nothing reads.
*/

std::vector<std::string> unknown_environment_variable_warnings();
