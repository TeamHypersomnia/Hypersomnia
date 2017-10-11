#pragma once
#include "augs/misc/timing/stepped_timing.h"

#include "game/transcendental/cosmos_common_state.h"

namespace augs {
	struct introspection_access;
}

class cosmos_metadata {
	// GEN INTROSPECTOR class cosmos_metadata
	friend class cosmos;
	friend struct augs::introspection_access;

	augs::delta delta = augs::delta::steps_per_second(60);
	augs::stepped_timestamp now = 0;

#if COSMOS_TRACKS_GUIDS
	entity_guid next_entity_guid = 1;
#endif
public:
	cosmos_common_state global;
	// END GEN INTROSPECTOR
};