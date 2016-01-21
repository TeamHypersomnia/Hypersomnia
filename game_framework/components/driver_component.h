#pragma once
#include "entity_system/entity.h"

namespace components {
	struct driver {
		augs::entity_id owned_vehicle;

		float force_towards_owned_wheel = 8000.f;
		float distance_when_force_easing_starts = 10.f;

		// float standard_linear_damping = 20.f;
		// float linear_damping_while_driving = 5.f;

		float power_of_force_easing_multiplier = 2.f;

		float density_while_driving = 0.2f;
		float standard_density = 0.6f;
	};
}