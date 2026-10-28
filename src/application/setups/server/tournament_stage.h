#pragma once
#include <string>
#include <algorithm>
#include "augs/string/typesafe_sprintf.h"

/*
	A tournament's stage: the rounds left to its final - 0 the final, 1 the semifinals, 2 the quarterfinals...
	(-1: not a tournament's match). The same words on the scoreboard, in the server browser and on the site.
*/

inline std::string tournament_stage_label(const int tournament_stage) {
	switch (tournament_stage) {
		case 0: return "FINALS";
		case 1: return "SEMIFINALS";
		case 2: return "QUARTERFINALS";
		default: return typesafe_sprintf("ROUND OF %x", 1 << std::clamp(tournament_stage + 1, 1, 16));
	}
}

/*
	What the players of a tournament's finished match are told on leaving it.
*/

inline std::string tournament_finished_text(const int tournament_stage) {
	if (tournament_stage == 0) {
		return "The tournament has finished.";
	}

	return "Match finished. You can watch the other matches.";
}
