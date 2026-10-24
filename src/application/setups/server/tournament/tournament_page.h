#pragma once
#include <cstdint>
#include <string>
#include <vector>

/*
	What a tournament's live page shows - /tournament.json, served by the tournament's server itself
	(see run_tournament.cpp) and drawn by detail/web/tournament_page.html.
*/

struct tournament_page_match {
	// GEN INTROSPECTOR struct tournament_page_match
	std::vector<std::string> team_a;
	std::vector<std::string> team_b;
	int score_a = -1;
	int score_b = -1;
	std::string status;
	std::string winner;
	std::string server_name;
	uint16_t port = 0;
	// END GEN INTROSPECTOR
};

struct tournament_page_round {
	// GEN INTROSPECTOR struct tournament_page_round
	std::string name;
	std::string arena;
	std::vector<tournament_page_match> matches;
	std::vector<std::vector<std::string>> byes;
	// END GEN INTROSPECTOR
};

struct tournament_page_state {
	// GEN INTROSPECTOR struct tournament_page_state
	std::string title;
	bool open_first_round = false;
	uint32_t players = 0;
	uint32_t team_size = 1;
	uint32_t current_round = 0;
	bool finished = false;
	std::vector<std::string> winner;
	std::vector<tournament_page_round> rounds;
	// END GEN INTROSPECTOR
};
