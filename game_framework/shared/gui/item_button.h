#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/shared/inventory_slot_id.h"

struct item_button : augs::gui::rect {
	augs::gui::appearance_detector detector;

	bool is_container_open = false;
	inventory_slot_id slot_id;
	augs::entity_id item;

	vec2 drag_offset;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};