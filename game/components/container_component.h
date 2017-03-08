#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory/inventory_slot.h"

#include "augs/misc/enum_associative_array.h"

namespace components {
	struct container {
		// GEN INTROSPECTOR struct components::container
		augs::enum_associative_array<slot_function, inventory_slot> slots;
		// END GEN INTROSPECTOR
	};
}
