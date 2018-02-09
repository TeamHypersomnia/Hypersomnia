#pragma once
#include "game/enums/entity_flag.h"
#include "augs/misc/enum/enum_boolset.h"

namespace invariants {
	struct flags {
		// GEN INTROSPECTOR struct invariants::flags
		augs::enum_boolset<entity_flag> values;
		// END GEN INTROSPECTOR
	};
}