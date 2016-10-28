#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory_slot_id.h"
#include "game/detail/gui/gui_element_location.h"

#include "augs/padding_byte.h"

struct slot_button : game_gui_rect_node<slot_button> {
	slot_button() {

	}

	bool houted_after_drag_started = true;
	padding_byte pad[3];

	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	//void perform_logic_step(augs::gui::rect_world&);
	//
	//void draw_triangles(draw_info);
	//void consume_gui_event(event_info);

	template <class C, class L>
	static void for_each_child(C context, const gui_element_location& this_id, L generic_call) {
		if (this_id.is<slot_button_for_inventory_slot_location>()) {
			const auto& slot_handle = context.get_step().get_cosmos()[this_id.get<slot_button_for_inventory_slot_location>().slot_id];

			for (const auto& i : slot_handle.get_items_inside()) {
				gui_element_location child_location;
				child_location.set(item_button_for_item_component_location{ i.get_id() });

				generic_call(i.get<components::item>().button, child_location);
			}
		}
	}
};

slot_button& get_meta(inventory_slot_id);
