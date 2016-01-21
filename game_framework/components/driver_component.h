#pragma once
#include "entity_system/entity.h"

namespace components {
	struct driver {
		augs::entity_id owned_vehicle;

		float force_towards_owned_wheel = 8000.f;
		float distance_when_force_easing_starts = 10.f;
	};
}