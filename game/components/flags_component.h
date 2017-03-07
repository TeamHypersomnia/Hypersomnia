#pragma once
#include <bitset>
#include "game/enums/entity_flag.h"
#include "augs/misc/enum_bitset.h"

namespace components {
	struct flags {
		// GEN INTROSPECTOR components::flags
		augs::enum_bitset<entity_flag> bit_flags;
		// END GEN INTROSPECTOR
	};
}