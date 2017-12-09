#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/transcendental/entity_id.h"

namespace augs {
	struct introspection_access;
}

class cosmos_meta {
	// GEN INTROSPECTOR class cosmos_meta
	friend class cosmos;
	friend augs::introspection_access;

	augs::delta delta = augs::delta::steps_per_second(60);
	augs::stepped_timestamp now = 0;

	entity_guid next_entity_guid = 1;
	// END GEN INTROSPECTOR
};