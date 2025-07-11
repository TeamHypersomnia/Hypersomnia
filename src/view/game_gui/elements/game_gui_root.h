#pragma once
#include <cstddef>
#include "view/game_gui/game_gui_element_location.h"
#include "view/game_gui/elements/item_button.h"

class game_gui_root : public game_gui_rect_node {
public:
	game_gui_root();

	template <class C, class gui_element_id, class L>
	static void for_each_child(
		const C context, 
		const gui_element_id, 
		L generic_call
	) {
		const auto handle = context.get_subject_entity();

		if (!handle.template has<components::item_slot_transfers>()) {
			return;
		}

		item_button::for_each_child(context, item_button_in_item{ handle.get_id() }, generic_call);

		context(drag_and_drop_target_drop_item_in_character_gui(), [&](const auto& dereferenced) {
			generic_call(dereferenced);
		});

		auto& character_gui = context.get_character_gui();

		for (std::size_t i = 0; i < character_gui.hotbar_buttons.size(); ++i) {
			hotbar_button_in_character_gui child_location;
			child_location.index = static_cast<int>(i);
			generic_call(make_dereferenced_location(&character_gui.hotbar_buttons[i], child_location));
		}

		for (std::size_t i = 0; i < character_gui.action_buttons.size(); ++i) {
			action_button_in_character_gui child_location;
			child_location.index = static_cast<int>(i);
			generic_call(make_dereferenced_location(&character_gui.action_buttons[i], child_location));
		}

		for (std::size_t i = 0; i < character_gui.value_bars.size(); ++i) {
			value_bar_in_character_gui child_location;
			child_location.vertical_index = static_cast<unsigned>(i);
			generic_call(make_dereferenced_location(&character_gui.value_bars[i], child_location));
		}
	}
};