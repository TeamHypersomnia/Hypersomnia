#pragma once
#include "augs/misc/enum/enum_array.h"

struct items_of_slots_cache {
	static constexpr bool is_cache = true;

	augs::enum_array<std::vector<entity_id>, slot_function> tracked_children;
};
