#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/enums/faction_type.h"
#include "game/modes/mode_player_id.h"
#include "game/detail/damage_origin.h"
#include "game/detail/economy/money_type.h"

struct arena_mode_win {
	// GEN INTROSPECTOR struct arena_mode_win
	augs::stepped_clock when;
	faction_type winner = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR

	bool was_set() const {
		return winner != faction_type::SPECTATOR;
	}
};

struct arena_mode_knockout {
	// GEN INTROSPECTOR struct arena_mode_knockout
	augs::stepped_clock when;

	mode_player_id knockouter;
	mode_player_id assist;
	mode_player_id victim;

	damage_origin origin;
	// END GEN INTROSPECTOR
};

struct arena_mode_award {
	// GEN INTROSPECTOR struct arena_mode_award
	augs::stepped_clock when;

	mode_player_id awarded_player;
	money_type amount;
	// END GEN INTROSPECTOR
};

using arena_mode_knockouts_vector = std::vector<arena_mode_knockout>;
using arena_mode_awards_vector = std::vector<arena_mode_award>;
