#pragma once
#include <variant>
#include <optional>

#include "augs/gui/dereferenced_location.h"
#include "game/detail/inventory/item_transfer_result.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "view/game_gui/elements/item_button.h"

struct unfinished_drag_of_item {
	hotbar_button_in_character_gui source_hotbar_button_id;

	entity_id item_id;
};

struct drop_for_item_slot_transfer {
	hotbar_button_in_character_gui source_hotbar_button_id;

	item_slot_transfer_request simulated_transfer;
	item_transfer_result result;

	std::string hint_text;
};

struct drop_for_hotbar_assignment {
	hotbar_button_in_character_gui source_hotbar_button_id;

	entity_id item_id;
	hotbar_button_in_character_gui assign_to;

	std::string hint_text;
};

using drag_and_drop_result = std::variant<
	drop_for_item_slot_transfer,
	drop_for_hotbar_assignment,
	unfinished_drag_of_item
>;

template <class C>
std::optional<drag_and_drop_result> prepare_drag_and_drop_result(
	const C context, 
	const game_gui_element_location held_rect_id, 
	const game_gui_element_location drop_target_rect_id
);

void drag_and_drop_callback(
	game_gui_context context, 
	const std::optional<drag_and_drop_result>&,
	const vec2i total_dragged_amount
);