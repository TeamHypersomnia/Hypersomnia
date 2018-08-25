#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/enums/faction_type.h"
#include "game/modes/mode_player_id.h"
#include "game/detail/damage_origin.h"

struct arena_mode_win {
	// GEN INTROSPECTOR struct arena_mode_win
	augs::stepped_clock when;
	faction_type winner = faction_type::NONE;
	// END GEN INTROSPECTOR

	bool was_set() const {
		return winner != faction_type::NONE;
	}
};

struct arena_mode_knockout {
	// GEN INTROSPECTOR struct arena_mode_knockout
	augs::stepped_clock when;

	mode_player_id knockouter;
	mode_player_id assist;
	mode_player_id victim;

	damage_cause cause;
	// END GEN INTROSPECTOR
};

using arena_mode_knockouts_vector = std::vector<arena_mode_knockout>;
