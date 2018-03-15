#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory/inventory_slot.h"

#include "augs/misc/enum/enum_map.h"

namespace invariants {
	struct container {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::container
		augs::enum_map<slot_function, inventory_slot> slots;
		// END GEN INTROSPECTOR
	};
}
