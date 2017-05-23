#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

using entity_name_type = std::wstring;
using fixed_entity_name_type = augs::constant_size_wstring<ENTITY_NAME_LENGTH>;

static_assert(fixed_entity_name_type::array_size % 4 == 0, "Wrong entity name padding");

namespace components {
	struct name;
}