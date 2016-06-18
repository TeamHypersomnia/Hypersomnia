#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/inventory_slot_id.h"

class item_button : public augs::gui::rect{
	void draw_dragged_ghost_inside(draw_info in);
	void draw_complete_with_children(draw_info in);

	rects::ltrb<float> iterate_children_attachments(bool draw = false, std::vector<vertex_triangle>* target = nullptr, augs::rgba col = augs::white);
	rects::ltrb<float> with_attachments_bbox;
public:
	item_button(rects::xywh<float> rc = rects::xywh<float>());

	bool is_being_wholely_dragged_or_pending_finish(augs::gui::gui_world& gr);

	augs::gui::text_drawer charges_caption;
	void get_member_children(std::vector<augs::gui::rect_id>&) final;

	entity_id gui_element_entity;
	
	bool is_inventory_root();

	augs::gui::appearance_detector detector;

	bool is_container_open = false;

	bool started_drag = false;

	entity_id item;

	vec2i drag_offset_in_item_deposit;

	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;

	void draw_grid_border_ghost(draw_info in);
	void draw_complete_dragged_ghost(draw_info);

	void consume_gui_event(event_info) final;

	void draw_proc(draw_info, bool draw_inside,
		bool draw_border,
		bool draw_connector,
		bool decrease_alpha,
		bool decrease_border_alpha = false,
		bool draw_container_opened_mark = false,
		bool draw_charges = true);
};

item_button& get_meta(entity_id);
