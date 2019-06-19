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

enum class general_gui_intent_type {
	// GEN INTROSPECTOR enum class general_gui_intent_type
	INVALID,

	TOGGLE_MOUSE_CURSOR,
	SWITCH_WEAPON_LASER,

	CHOOSE_TEAM,

	OPEN_SCOREBOARD,

	OPEN_BUY_MENU,
	OPEN_RCON_MENU,

	OPEN_CHAT,
	OPEN_TEAM_CHAT,

	SPECTATE_PREV,
	SPECTATE_NEXT,

	CLEAR_DEBUG_LINES,

	COUNT
	// END GEN INTROSPECTOR
};

using app_intent_map = augs::enum_map<
	augs::event::keys::key, 
	app_intent_type
>;

using general_gui_intent_map = augs::enum_map<
	augs::event::keys::key, 
	general_gui_intent_type
>;

struct general_gui_intent_input {
	const general_gui_intent_map& controls;
	const augs::event::state& common_input_state;
	const augs::event::change e;
};
