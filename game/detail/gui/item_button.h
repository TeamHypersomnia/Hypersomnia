#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory_slot_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/resources/manager.h"

#include "augs/padding_byte.h"
#include "game/resources/manager.h"

#include "gui_context.h"

struct item_button : game_gui_rect_node {
	typedef augs::gui::draw_info draw_info;
	typedef game_gui_rect_node base;

	typedef item_button_location location;
	typedef location_and_pointer<item_button> this_pointer;
	typedef location_and_pointer<const item_button> const_this_pointer;

	augs::gui::appearance_detector detector;
	rects::ltrb<float> with_attachments_bbox;

	bool is_container_open = false;
	bool started_drag = false;
	padding_byte pad[2];

	vec2i drag_offset_in_item_deposit;

	static rects::ltrb<float> iterate_children_attachments(
		const const_gui_context& context,
		const const_this_pointer& this_id,
		const bool draw = false,
		std::vector<vertex_triangle>* target = nullptr,
		const augs::rgba col = augs::white
	);

	struct drawing_flags {
		bool draw_inside = false;
		bool draw_border = false;
		bool draw_connector = false;
		bool decrease_alpha = false;
		bool decrease_border_alpha = false;
		bool draw_container_opened_mark = false;
		bool draw_charges = true;
		vec2 absolute_xy_offset;
	};

	item_button(rects::xywh<float> rc = rects::xywh<float>());

	template <class C, class gui_element_id, class L>
	static void for_each_child(C context, const gui_element_id& this_id, L generic_call) {
		const auto container = context.get_step().get_cosmos()[this_id.get_location().item_id];

		auto* maybe_container = container.find<components::container>();

		if (maybe_container) {
			for (auto& s : maybe_container->slots) {
				generic_call(make_location_and_pointer(&s.second.button, slot_button::location{ s.first }));
			}
		}
	}

	static void draw_dragged_ghost_inside(const viewing_gui_context& context, const const_this_pointer& this_id, augs::gui::draw_info in);
	static void draw_complete_with_children(const viewing_gui_context&, const const_this_pointer& this_id, augs::gui::draw_info in);

	static bool is_being_wholely_dragged_or_pending_finish(const const_gui_context&, const const_this_pointer& this_id);

	static void consume_gui_event(const logic_gui_context&, const this_pointer& this_id, const augs::gui::event_info e);
	static void perform_logic_step(const logic_gui_context&, const this_pointer& this_id);

	static bool is_inventory_root(const const_gui_context&, const const_this_pointer& this_id);
	static void draw_triangles(const viewing_gui_context&, const const_this_pointer& this_id, draw_info);
	static void draw_grid_border_ghost(const viewing_gui_context&, const const_this_pointer&, draw_info in);
	static void draw_complete_dragged_ghost(const viewing_gui_context&, const const_this_pointer&, draw_info);

	static void draw_proc(const viewing_gui_context&, const const_this_pointer&, draw_info, const drawing_flags&);
};