#pragma once
#include "augs/gui/rect.h"
#include "entity_system/entity.h"

struct slot_rect : augs::graphics::gui::rect {

};

struct item_rect : augs::graphics::gui::rect {
	bool is_container_open = false;
};

namespace components {
	struct gui_element {
		enum special_function {
			NONE,
			GUI_CROSSHAIR

		} element_type = NONE;

		//struct slot_metadata {
		//	bool is_open = false;
		//};
		//
		//struct item_metadata {
		//	augs::entity_id id;
		//};
		//
		//std::unordered_map<slot_metadata, rect> slot_metadata;
		//std::unordered_map<item_metadata, rect> item_metadata;
	};
}