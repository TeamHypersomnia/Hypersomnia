#pragma once
#include "game/build_settings.h"
#include "game/transcendental/entity_id.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct guid {
		static constexpr bool is_fundamental = true;
		friend class cosmos;
		friend struct augs::introspection_access;
private:
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