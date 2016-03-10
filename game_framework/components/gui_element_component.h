#pragma once
#include "augs/gui/rect.h"
#include "entity_system/entity.h"
#include <map>

#include "../detail/inventory_slot_id.h"
#include "augs/gui/appearance_detector.h"

#include "../detail/gui/item_button.h"
#include "../detail/gui/slot_button.h"

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