#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory_slot.h"

#include "augs/misc/enum_associative_array.h"

namespace components {
	struct container {
		augs::enum_associative_array<slot_function, inventory_slot> slots;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(slots));
		}
	};
}
