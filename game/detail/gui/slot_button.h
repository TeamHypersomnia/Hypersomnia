#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory_slot_id.h"
#include "game/detail/gui/gui_element_location.h"

#include "augs/padding_byte.h"

struct slot_button : game_gui_rect_leaf<slot_button> {
	slot_button() {

	}

	entity_id gui_element_entity;

	bool houted_after_drag_started = true;
	padding_byte pad[3];

	inventory_slot_id slot_id;
	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	//void perform_logic_step(augs::gui::rect_world&);
	//
	//void draw_triangles(draw_info);
	//void consume_gui_event(event_info);
};

slot_button& get_meta(inventory_slot_id);
