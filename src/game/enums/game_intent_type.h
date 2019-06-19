#pragma once
#include <vector>
#include "augs/misc/enum/enum_map.h"

#include "augs/misc/basic_input_intent.h"
#include "augs/misc/basic_input_motion.h"

#include "augs/window_framework/event.h"

enum class game_motion_type {
	// GEN INTROSPECTOR enum class game_motion_type
	MOVE_CROSSHAIR,

	COUNT
	// END GEN INTROSPECTOR
};

template <class T>
using per_game_motion_t = augs::enum_map<game_motion_type, T>;

enum class game_intent_type {
	// GEN INTROSPECTOR enum class game_intent_type
	CROSSHAIR_PRIMARY_ACTION,
	CROSSHAIR_SECONDARY_ACTION,

	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,

	DASH,
	SPRINT,
	PICK_TOUCHING_ITEMS,

	USE,
	RELOAD,

	DROP,
	THROW,

	THROW_ANY_FORCE,
	THROW_ANY_PED,
	THROW_ANY_INTERFERENCE,
	THROW_ANY_FLASH,

	THROW_ANY_MELEE,
	THROW_ANY_TWO_MELEES,

	DROP_SECONDARY,
	THROW_SECONDARY,

	WIELD_BOMB,

	SWITCH_CROSSHAIR_CHASE_TYPE,

	WALK,

	COUNT
	// END GEN INTROSPECTOR
};

using game_intent_map = augs::enum_map<
	augs::event::keys::key, 
	game_intent_type
>;

using game_intent = basic_input_intent<game_intent_type>;

using game_motion = basic_input_motion<game_motion_type, vec2>;
using raw_game_motion = basic_input_motion<game_motion_type, basic_vec2<short>>;

using game_motion_offset_type = decltype(game_motion::offset);

using game_intents = std::vector<game_intent>;

using raw_game_motions = std::vector<raw_game_motion>;

using game_motions = per_game_motion_t<game_motion_offset_type>;
using accumulated_motions = per_game_motion_t<raw_game_motion>;
