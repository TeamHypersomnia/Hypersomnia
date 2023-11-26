#pragma once
#include "game/common_state/entity_name_str.h"

namespace invariants {
	struct text_details {
		static constexpr bool allow_nontriviality = true;

		// GEN INTROSPECTOR struct invariants::text_details
		entity_name_str resource_id;
		entity_name_str name;
		entity_name_str description;
		// END GEN INTROSPECTOR
	};
}
