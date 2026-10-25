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

	Listed teams are nicknames ("alice" - let in by that nickname), or nicknames with account ids
	("alice=0x8aed..." - let in by the account, shown as alice): teams then holds the ids, team_nicknames the names.
*/

struct tournament_config {
	// GEN INTROSPECTOR struct tournament_config
	tournament_skill_source skill_level_source = tournament_skill_source::ORDER;
	bool skill_by_match_time = true;

	bool byes_to_strongest = true;
	tournament_matchup_scheme matchup_scheme = tournament_matchup_scheme::STRONGEST_VS_STRONGEST;

	std::vector<std::vector<std::string>> teams;
	std::vector<std::vector<std::string>> team_nicknames;
	std::vector<std::string> arenas;
	std::vector<std::string> additional_spectators;

	uint32_t open_players = 0;
	uint32_t open_team_size = 1;
	// END GEN INTROSPECTOR

	/*
		Join timeout: how long a match waits for all its players - then it's decided without them
		(server_join_timeout.hpp). 0: it waits. Not part of the tournament's identity (compute_hash) -
		changed on a restart, the tournament carries on.
	*/

	uint32_t join_timeout_secs = 10 * 60;

	/*
		The tournament's name (--tournament-name) - on its page and in its results. Part of its identity (compute_hash):
		the same settings under a new name are a new tournament.
	*/

	std::string name;

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

	/*
		How many rounds are left for this many teams - the byes fill them up to 2, 4, 8... once, in the next round.
	*/

	static uint32_t rounds_for_teams(uint32_t teams);

	/*
		The arena of a round with this many rounds left, this one included: counted from the end of the list,
		so the last arena is always the final's - also when fewer came than the bracket was made for.
	*/

	const std::string& arena_for_rounds_left(uint32_t rounds_left) const;

	std::string compute_hash() const;
};
