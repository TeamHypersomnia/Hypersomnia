#pragma once
#include "game/messages/gui_intents.h"

class item_button;

struct drag_and_drop_result {
	messages::gui_item_transfer_intent intent;
	item_button* dragged_item = nullptr;
	bool possible_target_hovered = false;
	item_transfer_result result;

	bool will_item_be_disposed();
	bool will_drop_be_successful();
	std::wstring tooltip_text;
};