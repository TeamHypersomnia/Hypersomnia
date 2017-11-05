#pragma once
#include "game/transcendental/entity_id.h"

namespace augs {
	struct introspection_access;
}

class cosmos;

namespace components {
	struct guid {
		static constexpr bool is_fundamental = true;
private:
		friend ::cosmos;
		friend augs::introspection_access;

		// GEN INTROSPECTOR struct components::guid
		entity_guid value = 0;
		// END GEN INTROSPECTOR
	public:
		auto get_value() const {
			return value;
		}
	};
}