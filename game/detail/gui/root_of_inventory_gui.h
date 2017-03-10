#pragma once
#include "game/detail/gui/game_gui_element_location.h"

class root_of_inventory_gui : public game_gui_rect_node {
public:
	root_of_inventory_gui(const vec2 screen_size);

	template <class C, class gui_element_id, class L>
	static void for_each_child(const C context, const gui_element_id this_id, L generic_call) {
		const auto handle = context.get_gui_element_entity();

		// we do not dereference the gui element's entity location because it is possibly not an item;
		// however it should be a container so we call the callback on the element's children
		// i.e. the player has a gui element component and container component but not an item component.
		item_button::for_each_child(context, item_button_in_item{ handle.get_id() }, generic_call);

		context(drag_and_drop_target_drop_item_in_character_gui(), [&](const auto& dereferenced) {
			generic_call(dereferenced);
		});

		for (size_t i = 0; i < context.get_character_gui().hotbar_buttons.size(); ++i) {
			hotbar_button_in_character_gui child_location;
			child_location.index = static_cast<int>(i);
			generic_call(make_dereferenced_location(&context.get_character_gui().hotbar_buttons[i], child_location));
		}

		for (size_t i = 0; i < context.get_character_gui().action_buttons.size(); ++i) {
			action_button_in_character_gui child_location;
			child_location.index = static_cast<int>(i);
			generic_call(make_dereferenced_location(&context.get_character_gui().action_buttons[i], child_location));
		}

		for (size_t i = 0; i < context.get_character_gui().sentience_meter_bars.size(); ++i) {
			sentience_meter_bar_in_character_gui child_location;
			child_location.type = static_cast<sentience_meter_type>(i);
			generic_call(make_dereferenced_location(&context.get_character_gui().sentience_meter_bars[i], child_location));
		}

		for (size_t i = 0; i < context.get_character_gui().perk_meters.size(); ++i) {
			perk_meter_bar_in_character_gui child_location;
			child_location.type = static_cast<perk_meter_type>(i);
			generic_call(make_dereferenced_location(&context.get_character_gui().perk_meters[i], child_location));
		}
	}
};