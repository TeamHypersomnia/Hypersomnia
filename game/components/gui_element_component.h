#pragma once
#include "augs/gui/rect.h"
#include "game/entity_id.h"
#include <map>

#include "game/detail/inventory_slot_id.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"

namespace components {
	struct gui_element {
		enum special_function {
			NONE,
			GUI_CROSSHAIR
		} element_type = NONE;

		std::map<inventory_slot_id, slot_button> slot_metadata;
		std::map<entity_id, item_button> item_metadata;

		std::map<inventory_slot_id, slot_button> removed_slot_metadata;
		std::map<entity_id, item_button> removed_item_metadata;
	};
}