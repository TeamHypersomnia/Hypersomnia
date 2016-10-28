#pragma once
#include "augs/gui/rect.h"
#include "game/detail/gui/gui_element_location.h"

class root_of_inventory_gui : public game_gui_rect_node<root_of_inventory_gui> {
public:
	root_of_inventory_gui() {
		unset_flag(augs::gui::flag::CLIP);
	}

	template <class C, class L>
	static void for_each_child(C context, const gui_element_location& this_id, L generic_call) {
		const auto& handle = context.get_gui_element_entity();

		gui_element_location gui_entity_location;
		gui_entity_location.set(item_button_for_item_component_location{ handle.get_id() });

		item_button::for_each_child(context, gui_entity_location, generic_call);

		for (auto i = gui_element_internal::DROP_ITEM_ICON; i < gui_element_internal::COUNT; i = gui_element_internal(int(i) + 1)) {
			gui_element_location internal_location;
			internal_of_gui_element_component_location loc;
			loc.element = i;
			internal_location.set(loc);

			context(internal_location, [&](auto& internal_element) {
				generic_call(internal_element, internal_location);
			});
		}
	}
};