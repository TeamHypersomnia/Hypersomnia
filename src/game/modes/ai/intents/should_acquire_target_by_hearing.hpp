#pragma once
#include "game/enums/faction_type.h"

/*
	Stateless calculation of whether to fully acquire a target just from hearing.
	
	For Metropolis only, returns true when the bomb is planted, otherwise false.
	This causes bots to immediately engage enemies they hear near a bomb site
	when defending.
*/

inline bool should_acquire_target_by_hearing(
	const faction_type bot_faction,
	const bool bomb_planted
) {
	if (bot_faction == faction_type::METROPOLIS && bomb_planted) {
		return true;
	}

	return false;
}
