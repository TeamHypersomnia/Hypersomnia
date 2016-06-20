#pragma once
#include <unordered_map>
#include "game/globals/slot_function.h"

struct inventory_slot;

namespace components {
	struct container {
		std::unordered_map<slot_function, inventory_slot> slots;
	};
}
