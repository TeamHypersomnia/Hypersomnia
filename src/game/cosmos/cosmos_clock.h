#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/cosmos/entity_id.h"

struct cosmos_clock {
	// GEN INTROSPECTOR struct cosmos_clock
	augs::delta delta = augs::delta::steps_per_second(60);
	augs::stepped_timestamp now = 0;

	entity_guid next_entity_guid = entity_guid::first();
	// END GEN INTROSPECTOR
};