#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/shared/inventory_slot_id.h"

struct item_button : augs::gui::rect {
	item_button(rects::xywh<float> rc = rects::xywh<float>());

	augs::gui::text_drawer charges_caption;
	void get_member_children(std::vector<augs::gui::rect_id>&) final;

	augs::entity_id gui_element_entity;
	
	bool is_inventory_root();

	augs::gui::appearance_detector detector;

	bool is_container_open = false;
	augs::entity_id item;

	vec2 user_drag_offset;

	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;

	void draw_proc(draw_info, bool dragged_ghost);
};

item_button& get_meta(augs::entity_id);
