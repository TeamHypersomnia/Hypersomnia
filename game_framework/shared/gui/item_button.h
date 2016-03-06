#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/shared/inventory_slot_id.h"

struct item_button : augs::gui::rect {
	item_button(rects::xywh<float> rc = rects::xywh<float>());
	
	augs::entity_id gui_element_entity;
	bool being_dragged = false;
	
	bool is_inventory_root();

	augs::gui::appearance_detector detector;

	bool is_container_open = false;
	augs::entity_id item;

	vec2 user_drag_offset;

	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};

item_button& get_meta(augs::entity_id);
