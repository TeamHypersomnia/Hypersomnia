#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory/inventory_slot.h"

#include "augs/misc/enum/enum_associative_array.h"

namespace invariants {
	struct container {
		// GEN INTROSPECTOR struct invariants::container
		augs::enum_associative_array<slot_function, inventory_slot> slots;
		// END GEN INTROSPECTOR
	};
}
