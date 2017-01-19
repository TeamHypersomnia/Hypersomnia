#pragma once
#include "augs/gui/dereferenced_location.h"
#include "game/messages/gui_intents.h"
#include "game/detail/item_transfer_result.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/inventory_utils.h"

struct unfinished_drag_of_item {
	hotbar_button_in_gui_element source_hotbar_button_id;

	entity_id item_id;
};

struct drop_for_item_slot_transfer {
	hotbar_button_in_gui_element source_hotbar_button_id;

	item_slot_transfer_request_data simulated_transfer;
	item_transfer_result result;

	augs::constant_size_wstring<32> hint_text;
};

struct drop_for_hotbar_assignment {
	hotbar_button_in_gui_element source_hotbar_button_id;

	entity_id item_id;
	hotbar_button_in_gui_element assign_to;

	augs::constant_size_wstring<20> hint_text;
};

typedef augs::trivial_variant<
	drop_for_item_slot_transfer,
	drop_for_hotbar_assignment,
	unfinished_drag_of_item
> drag_and_drop_result;

template <class C>
drag_and_drop_result prepare_drag_and_drop_result(
	C context, 
	const game_gui_element_location held_rect_id, 
	const game_gui_element_location drop_target_rect_id
);

void drag_and_drop_callback(
	logic_gui_context context, 
	const drag_and_drop_result&,
	const vec2i total_dragged_amount
);