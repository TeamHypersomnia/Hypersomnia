#pragma once
#include "augs/gui/rect.h"

class root_of_inventory_gui : public game_gui_rect_node {
public:
	typedef root_of_inventory_gui_location location;

	root_of_inventory_gui() {
		unset_flag(augs::gui::flag::CLIP);
	}

	template <class C, class gui_element_id, class L>
	static void for_each_child(C context, const gui_element_id& this_id, L generic_call) {
		const auto& handle = context.get_gui_element_entity();

		context(item_button::location{ handle.get_id() }, [&](const auto& dereferenced) {
			generic_call(dereferenced);
		}); 

		context(drag_and_drop_target_drop_item::location(), [&](const auto& dereferenced) {
			generic_call(dereferenced);
		});
	}
};