#pragma once
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/basic_input_intent.h"

#include "augs/window_framework/event.h"

enum class game_gui_intent_type {
	// GEN INTROSPECTOR enum class game_gui_intent_type
	INVALID,

	HOLSTER,
	HOLSTER_SECONDARY,

	HOTBAR_BUTTON_0,
	HOTBAR_BUTTON_1,
	HOTBAR_BUTTON_2,
	HOTBAR_BUTTON_3,
	HOTBAR_BUTTON_4,
	HOTBAR_BUTTON_5,
	HOTBAR_BUTTON_6,
	HOTBAR_BUTTON_7,
	HOTBAR_BUTTON_8,
	HOTBAR_BUTTON_9,

	LAST_HOTBAR_SELECTION_SETUP,

	SPECIAL_ACTION_BUTTON_1,
	SPECIAL_ACTION_BUTTON_2,
	SPECIAL_ACTION_BUTTON_3,
	SPECIAL_ACTION_BUTTON_4,
	SPECIAL_ACTION_BUTTON_5,
	SPECIAL_ACTION_BUTTON_6,
	SPECIAL_ACTION_BUTTON_7,
	SPECIAL_ACTION_BUTTON_8,
	SPECIAL_ACTION_BUTTON_9,
	SPECIAL_ACTION_BUTTON_10,
	SPECIAL_ACTION_BUTTON_11,
	SPECIAL_ACTION_BUTTON_12,

	PREV_HOTBAR_ITEM,
	NEXT_HOTBAR_ITEM,

	COUNT
	// END GEN INTROSPECTOR
};

using game_gui_intent = basic_input_intent<game_gui_intent_type>;

using game_gui_intent_map = augs::enum_map<
	augs::event::keys::key,
	game_gui_intent_type
>;
