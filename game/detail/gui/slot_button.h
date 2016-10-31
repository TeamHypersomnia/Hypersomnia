#pragma once
#include "augs/gui/gui_event.h"
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory_slot_id.h"
#include "game/detail/gui/dispatcher_context.h"

#include "augs/padding_byte.h"

struct slot_button : game_gui_rect_node {
	typedef slot_button_location location;
	typedef location_and_pointer<slot_button> this_pointer;
	typedef location_and_pointer<const slot_button> const_this_pointer;

	bool houted_after_drag_started = true;
	padding_byte pad[3];

	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	slot_button();

	static void perform_logic_step(const dispatcher_context&, const this_pointer&);
	
	static void draw_triangles(const viewing_dispatcher_context&, const const_this_pointer&, augs::gui::draw_info);
	static void consume_gui_event(dispatcher_context&, const this_pointer&, const augs::gui::event_info info);

	template <class C, class gui_element_id, class L>
	static void for_each_child(C context, const gui_element_id& this_id, L generic_call) {
		const auto& slot_handle = context.get_step().get_cosmos()[this_id.get_location().slot_id];

		for (const auto& i : slot_handle.get_items_inside()) {
			generic_call(make_location_and_pointer(&i.get<components::item>().button, item_button::location{ i.get_id() }));
		}
	}
};

slot_button& get_meta(inventory_slot_id);