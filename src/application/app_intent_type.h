#pragma once
#include "augs/misc/enum/enum_map.h"
#include "augs/window_framework/event.h"
#include "game/enums/game_intent_type.h"

enum class app_intent_type {
	// GEN INTROSPECTOR enum class app_intent_type
	INVALID,

	SHOW_PERFORMANCE,
	SHOW_LOGS,

	TOGGLE_CINEMATIC_MODE,
	TOGGLE_STREAMER_MODE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class general_gui_intent_type {
	// GEN INTROSPECTOR enum class general_gui_intent_type
	INVALID,

	TOGGLE_WEAPON_LASER,

	CHAT,
	CHAT_COMMAND,
	TEAM_CHAT,
	BUY_MENU,
	SERVER_ADMIN_PANEL,
	EXECUTE_RCON_GAME_COMMANDS,

	CHOOSE_TEAM,

	SCOREBOARD,

	TOGGLE_MOUSE_CURSOR,
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
	const game_intent_map& game_controls;
	const augs::event::state& common_input_state;
	const augs::event::change e;
};
