#pragma once
#include "game/transcendental/entity_id.h"

namespace augs {
	struct introspection_access;
}

class cosmos_solvable;

namespace components {
	struct guid {
		static constexpr bool is_always_present = true;
	private:
		friend augs::introspection_access;
		friend class ::cosmos_solvable;

		// GEN INTROSPECTOR struct components::guid
		entity_guid value = 0;
		// END GEN INTROSPECTOR
	public:
		auto get_value() const {
			return value;
		}
	};
}