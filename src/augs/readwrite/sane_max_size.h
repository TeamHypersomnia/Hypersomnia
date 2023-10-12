#pragma once
#include "augs/network/network_types.h"
#include "augs/templates/type_map.h"

using sane_max_size_map = type_value_map<uint32_t,
	static_cast<uint32_t>(-1),

	type_uint32_pair<std::vector<arena_and_mode_identifier>, 1000>
>;
