#pragma once
#include "augs/gui/rect.h"
#include "entity_system/entity.h"
#include <map>

#include "../shared/inventory_slot_id.h"
#include "augs/gui/appearance_detector.h"

#include "../shared/gui/item_button.h"
#include "../shared/gui/slot_button.h"

namespace components {
	struct gui_element {
		enum special_function {
			NONE,
			GUI_CROSSHAIR

		} element_type = NONE;

		std::map<inventory_slot_id, slot_button> slot_metadata;
		std::map<augs::entity_id, item_button> item_metadata;
	};
}