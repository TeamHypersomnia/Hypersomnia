#pragma once
#include "augs/gui/rect.h"
#include "game/detail/gui/gui_element_location.h"

class root_of_inventory_gui : public game_gui_rect_node<root_of_inventory_gui> {
public:
	root_of_inventory_gui() {
		unset_flag(augs::gui::flag::CLIP);
	}

	template <class C, class L>
	static void for_each_child(C context, const gui_element_location& this_id, L polymorphic_call) {
		const auto& handle = context.get_gui_element_entity();
	}
};