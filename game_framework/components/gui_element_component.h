#pragma once
#include "augs/gui/rect.h"
#include "entity_system/entity.h"
#include <map>

#include "../shared/inventory_slot_id.h"
#include "augs/gui/appearance_detector.h"

struct slot_rect : augs::gui::rect {
	inventory_slot_id slot_id;

	augs::gui::appearance_detector detector;

	void draw_triangles(draw_info) final;
	void consume_gui_event(event_info) final;
};

struct item_rect : augs::gui::rect {
	bool is_container_open = false;
	inventory_slot_id slot_id;
	augs::entity_id item;

	vec2 drag_offset;
};

namespace components {
	struct gui_element {
		enum special_function {
			NONE,
			GUI_CROSSHAIR

		} element_type = NONE;

		std::map<inventory_slot_id, slot_rect> slot_metadata;
		std::map<augs::entity_id, item_rect> item_metadata;
	};
}