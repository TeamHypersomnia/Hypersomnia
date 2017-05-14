#pragma once
#include "game/enums/entity_flag.h"
#include "augs/misc/enum_boolset.h"

namespace components {
	struct flags {
		// GEN INTROSPECTOR struct components::flags
		augs::enum_boolset<entity_flag> values;
		// END GEN INTROSPECTOR
	};
}