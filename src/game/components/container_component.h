#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory/inventory_slot.h"

#include "augs/misc/enum/enum_map.h"

using container_slots_type = augs::enum_map<slot_function, inventory_slot>;

namespace invariants {
	struct container {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::container
		container_slots_type slots;
		// END GEN INTROSPECTOR
	};
}
