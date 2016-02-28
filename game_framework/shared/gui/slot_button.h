#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game_framework/shared/inventory_slot_id.h"

struct slot_button : augs::gui::rect {
	inventory_slot_id slot_id;

	augs::gui::appearance_detector detector;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};
