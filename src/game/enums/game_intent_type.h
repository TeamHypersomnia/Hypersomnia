#pragma once
#include <vector>
#include "augs/misc/enum/enum_map.h"

#include "augs/misc/basic_input_intent.h"
#include "augs/misc/basic_input_motion.h"

#include "augs/window_framework/event.h"

enum class game_motion_type {
	// GEN INTROSPECTOR enum class game_motion_type
	INVALID,

	MOVE_CROSSHAIR,

	COUNT
	// END GEN INTROSPECTOR
};

template <class T>
using per_game_motion_t = augs::enum_map<game_motion_type, T>;

enum class game_intent_type {
	// GEN INTROSPECTOR enum class game_intent_type
	INVALID,

	DROP,
	DROP_SECONDARY,

	THROW,
	THROW_SECONDARY,

	START_PICKING_UP_ITEMS,

	USE_BUTTON,

	SPACE_BUTTON,
	HAND_BRAKE,

	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,
	WALK,
	SPRINT,

	CROSSHAIR_PRIMARY_ACTION,
	CROSSHAIR_SECONDARY_ACTION,

	MELEE_BLOCK,

	RELOAD,

	SWITCH_LOOK,

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

using game_intents = std::vector<game_intent>;
using game_motions = std::vector<game_motion>;
using raw_game_motions = std::vector<raw_game_motion>;

using accumulated_motions = per_game_motion_t<raw_game_motion>;
