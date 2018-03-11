#pragma once
#include "game/common_state/entity_name_str.h"

namespace invariants {
	struct name {
		static constexpr bool allow_nontriviality = true;

		// GEN INTROSPECTOR struct invariants::name
		entity_name_str name;
		entity_name_str description;
		// END GEN INTROSPECTOR
	};
}
