#pragma once
#include "game/build_settings.h"
#include "game/transcendental/entity_id.h"

namespace components {
	struct guid {
#if COSMOS_TRACKS_GUIDS
		// GEN INTROSPECTOR struct components::guid
		entity_guid value = 0;
		// END GEN INTROSPECTOR
#endif
	};
}