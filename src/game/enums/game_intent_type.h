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
	SHOOT,
	SHOOT_SECONDARY,

	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,

	DASH,
	SPRINT,
	TOGGLE_SPRINT,

	WALK_SILENTLY,
	TOGGLE_WALK_SILENTLY,

	INTERACT,
	RELOAD,

	DROP,
	THROW,

	WIELD_BOMB,

	THROW_FORCE_GRENADE,
	THROW_PED_GRENADE,
	THROW_INTERFERENCE_GRENADE,
	THROW_FLASHBANG,

	THROW_KNIFE,
	THROW_TWO_KNIVES,

	SWITCH_CAMERA_MODE,

	DROP_SECONDARY,
	THROW_SECONDARY,

	TOGGLE_ZOOM_OUT,

	COUNT
	// END GEN INTROSPECTOR
};

using game_intent_map = augs::enum_map<
	augs::event::keys::key, 
	game_intent_type
>;


using game_intent = basic_input_intent<game_intent_type>;
using game_intents = std::vector<game_intent>;

using game_motion = basic_input_motion<game_motion_type, vec2>;

using raw_game_motion = basic_input_motion<game_motion_type, basic_vec2<short>>;
using raw_game_motion_offset_type = decltype(raw_game_motion::offset);
using raw_game_motion_vector = std::vector<raw_game_motion>;

using raw_game_motion_map = per_game_motion_t<raw_game_motion_offset_type>;
using accumulated_motions = per_game_motion_t<raw_game_motion>;
