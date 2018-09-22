#pragma once
#include "augs/misc/enum/enum_map.h"
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
	SWITCH_GAME_GUI_MODE,

	OPEN_BUY_MENU,
	OPEN_SCOREBOARD,

	CHOOSE_TEAM,

	COUNT
	// END GEN INTROSPECTOR
};

using app_intent_map = augs::enum_map<
	augs::event::keys::key, 
	app_intent_type
>;

using app_ingame_intent_map = augs::enum_map<
	augs::event::keys::key, 
	app_ingame_intent_type
>;

struct app_ingame_intent_input {
	const app_ingame_intent_map& controls;
	const augs::event::state& common_input_state;
	const augs::event::change e;
};
