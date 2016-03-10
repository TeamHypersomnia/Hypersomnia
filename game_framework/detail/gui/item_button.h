#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/detail/inventory_slot_id.h"

struct item_button : augs::gui::rect {
private:
	void draw_dragged_ghost_inside(draw_info in);
	void draw_complete_with_children(draw_info in);
public:
	item_button(rects::xywh<float> rc = rects::xywh<float>());

	augs::gui::text_drawer charges_caption;
	void get_member_children(std::vector<augs::gui::rect_id>&) final;

	augs::entity_id gui_element_entity;
	
	bool is_inventory_root();

	augs::gui::appearance_detector detector;

	bool is_container_open = false;

	bool started_drag = false;

	augs::entity_id item;

	vec2i drag_offset_in_item_deposit;

	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;

	void draw_grid_border_ghost(draw_info in);
	void draw_complete_dragged_ghost(draw_info);

	void consume_gui_event(event_info) final;

	void draw_proc(draw_info, bool draw_inside, bool draw_border, bool draw_connector, bool decrease_alpha, bool decrease_border_alpha = false);
};

item_button& get_meta(augs::entity_id);
