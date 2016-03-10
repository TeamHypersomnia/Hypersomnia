#pragma once
#include <unordered_map>
#include "../detail/inventory_slot.h"

namespace components {
	struct container {
		std::unordered_map<slot_function, inventory_slot> slots;
	};
}
