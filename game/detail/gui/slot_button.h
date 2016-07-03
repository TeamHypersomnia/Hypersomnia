#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/text_drawer.h"

#include "game/detail/inventory_slot_id.h"

struct slot_button : augs::gui::rect {
	slot_button();

	entity_id gui_element_entity;

	augs::gui::text_drawer space_caption;
	void get_member_children(std::vector<augs::gui::rect_id>&) const final;

	bool houted_after_drag_started = true;

	inventory_slot_id slot_id;
	vec2i slot_relative_pos;
	vec2i user_drag_offset;

	augs::gui::appearance_detector detector;
	
	void perform_logic_step(augs::gui::gui_world&) final;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};

slot_button& get_meta(inventory_slot_id);
