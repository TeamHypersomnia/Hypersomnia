#pragma once
#include "augs/misc/enum/enum_associative_array.h"
#include "augs/misc/basic_input_intent.h"

#include "augs/window_framework/event.h"

enum class app_intent_type {
	// GEN INTROSPECTOR enum class app_intent_type
	INVALID,

	SWITCH_DEVELOPER_CONSOLE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class app_ingame_intent_type {
	// GEN INTROSPECTOR enum class app_ingame_intent_type
	INVALID,

	CLEAR_DEBUG_LINES,
	SWITCH_WEAPON_LASER,
	SWITCH_GAME_GUI_ACTIVE,
	SWITCH_CHARACTER,

	COUNT
	// END GEN INTROSPECTOR
};

#if 0
using app_intent = basic_input_intent<app_intent_type>;
#endif

using app_intent_map = augs::enum_associative_array<
	augs::event::keys::key, 
	app_intent_type
>;

using app_ingame_intent_map = augs::enum_associative_array<
	augs::event::keys::key, 
	app_ingame_intent_type
>;
