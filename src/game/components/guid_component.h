#pragma once
#include "game/build_settings.h"
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

#if COSMOS_TRACKS_GUIDS
		// GEN INTROSPECTOR struct components::guid
		entity_guid value = 0;
		// END GEN INTROSPECTOR
#endif
	public:
		auto get_value() const {
			return value;
		}
	};
}