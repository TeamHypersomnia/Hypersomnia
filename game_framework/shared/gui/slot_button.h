#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/shared/inventory_slot_id.h"

struct slot_button : augs::gui::rect {
	augs::entity_id gui_element_entity;

	inventory_slot_id slot_id;
	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};

slot_button& get_meta(inventory_slot_id);
