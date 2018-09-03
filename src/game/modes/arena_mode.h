#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/misc/enum/enum_array.h"
#include "game/enums/battle_event.h"
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

enum class scoreboard_icon_type {
	// GEN INTROSPECTOR enum class scoreboard_icon_type
	DEATH_ICON,
	UNCONSCIOUS_ICON,
	NO_AMMO_ICON,
	BOMB_ICON,
	DEFUSE_KIT_ICON,

	COUNT
	// END GEN INTROSPECTOR
};

struct arena_mode_view_vars {
	// GEN INTROSPECTOR struct arena_mode_view_vars
	per_faction_t<per_faction_t<assets::sound_id>> win_sounds;
	per_faction_t<augs::enum_array<assets::sound_id, battle_event>> event_sounds;

	per_faction_t<assets::image_id> logos;
	per_faction_t<assets::image_id> square_logos;
	bool show_money_of_opponents = false;
	bool show_info_icons_of_opponents = false;

	augs::enum_array<assets::image_id, scoreboard_icon_type> icons;
	// END GEN INTROSPECTOR
};

using arena_mode_knockouts_vector = std::vector<arena_mode_knockout>;
using arena_mode_awards_vector = std::vector<arena_mode_award>;
