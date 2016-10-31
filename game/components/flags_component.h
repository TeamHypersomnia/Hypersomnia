#pragma once
#include <bitset>
#include "game/enums/entity_flag.h"
#include "augs/misc/enum_bitset.h"

namespace components {
	struct flags {
		augs::enum_bitset<entity_flag> bit_flags;
	};
}