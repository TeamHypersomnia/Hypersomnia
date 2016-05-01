#pragma once
#include "message.h"

namespace messages {
	struct health_event : message {
		enum result_type {
			NONE,
			SHIELD_DESTRUCTION,
			DEATH,
			LOSS_OF_CONSCIOUSNESS
		} special_result = result_type::NONE;

		enum target_type {
			INVALID,
			AIM,
			SHIELD,
			CONSCIOUSNESS,
			HEALTH
		} target = target_type::INVALID;

		augs::entity_id spawned_remnants;

		vec2 point_of_impact;
		vec2 impact_velocity;

		float ratio_to_maximum_value = 1.f;
		float objective_amount = 0.f;
		float effective_amount = 0.f;
	};
}