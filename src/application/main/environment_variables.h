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
	  A flag given on the command line wins.
*/

struct environment_flag {
	const char* variable;
	const char* flag;
};

inline constexpr auto environment_flags = std::array {
	environment_flag { "HYPERSOMNIA_TOURNAMENT", "--tournament" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_TEAM_SIZE", "--tournament-team-size" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_TEAMS", "--tournament-teams" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_ARENAS", "--tournament-arenas" },
	environment_flag { "HYPERSOMNIA_TOURNAMENT_PAGE", "--tournament-page" }
};

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
