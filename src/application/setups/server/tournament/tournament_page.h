#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/*
	A tournament as anyone may read it - /tournament.json, served by the tournament's server itself (see run_tournament.cpp),
	drawn by detail/web/tournament_page.html, and posted once to the results webhook when it's over:
	the rounds from the final down, each match's teams and score, the settings it's played with last.
*/

struct tournament_page_player {
	// GEN INTROSPECTOR struct tournament_page_player
	std::string nickname;
	std::string account;
	// END GEN INTROSPECTOR
};

using tournament_page_team = std::vector<tournament_page_player>;

struct tournament_page_match {
	// GEN INTROSPECTOR struct tournament_page_match
	std::string status;
	std::string winner;
	tournament_page_team team_a;
	tournament_page_team team_b;
	std::optional<int> score_a;
	std::optional<int> score_b;
	std::optional<std::string> server_name;
	std::optional<uint16_t> port;
	// END GEN INTROSPECTOR
};

struct tournament_page_bracket_round {
	// GEN INTROSPECTOR struct tournament_page_bracket_round
	std::string name;
	std::string state;
	std::string map;
	std::vector<tournament_page_match> matches;
	std::vector<tournament_page_team> byes;
	std::optional<uint64_t> deadline;
	std::optional<uint32_t> grace_secs;
	// END GEN INTROSPECTOR
};

struct tournament_page_config {
	// GEN INTROSPECTOR struct tournament_page_config
	std::string who_participates;
	uint32_t team_size = 1;
	uint32_t teams = 0;
	std::vector<std::string> maps;
	uint32_t round_1_max_wait_minutes = 0;
	uint32_t next_rounds_max_wait_minutes = 0;
	// END GEN INTROSPECTOR
};

struct tournament_page_state {
	// GEN INTROSPECTOR struct tournament_page_state
	std::optional<std::string> name;
	uint64_t now = 0;
	std::string state;
	tournament_page_team winner_team;
	std::vector<tournament_page_bracket_round> rounds;
	tournament_page_config config;
	// END GEN INTROSPECTOR
};
