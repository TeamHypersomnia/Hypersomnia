#pragma once
#include "game/enums/entity_flag.h"
#include "augs/misc/enum/enum_boolset.h"

namespace components {
	struct flags {
		static constexpr bool is_fundamental = true;

		// GEN INTROSPECTOR struct components::flags
		augs::enum_boolset<entity_flag> values;
		// END GEN INTROSPECTOR
	};
}