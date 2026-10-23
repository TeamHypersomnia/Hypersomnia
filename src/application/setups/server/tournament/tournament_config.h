#pragma once
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path_declaration.h"

struct cmd_line_params;

enum class tournament_skill_source {
	// GEN INTROSPECTOR enum class tournament_skill_source
	ORDER,
	RANDOM,
	ENDPOINT,

	COUNT
	// END GEN INTROSPECTOR
};

enum class tournament_matchup_scheme {
	// GEN INTROSPECTOR enum class tournament_matchup_scheme
	STRONGEST_VS_WEAKEST,
	STRONGEST_VS_STRONGEST,

	COUNT
	// END GEN INTROSPECTOR
};

/*
	A tournament, made from the flags (or the HYPERSOMNIA_TOURNAMENT* variables) - see from_params.
	Either the teams are listed (lines 1 and 2 play each other, 3 and 4...; the byes go to the first lines),
	or the first round is open: open_players come, open_team_size each, and they make the matches as they join.
*/

struct tournament_config {
	// GEN INTROSPECTOR struct tournament_config
	tournament_skill_source skill_level_source = tournament_skill_source::ORDER;
	bool skill_by_match_time = true;

	bool byes_to_strongest = true;
	tournament_matchup_scheme matchup_scheme = tournament_matchup_scheme::STRONGEST_VS_STRONGEST;

	std::vector<std::vector<std::string>> teams;
	std::vector<std::string> arenas;
	std::vector<std::string> additional_spectators;

	uint32_t open_players = 0;
	uint32_t open_team_size = 1;
	// END GEN INTROSPECTOR

	bool is_open() const {
		return teams.empty() && open_players > 0;
	}

	/*
		How many teams take part - the listed ones, or as many as the players coming make.
	*/

	uint32_t num_teams() const {
		return is_open() ? (open_players + open_team_size - 1) / open_team_size : static_cast<uint32_t>(teams.size());
	}

	/*
		Empty error: the tournament the flags describe. Otherwise what's wrong with them.
	*/

	static tournament_config from_params(const cmd_line_params&, std::string& error);

	std::string compute_hash() const;
};
