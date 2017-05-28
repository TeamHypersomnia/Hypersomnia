#pragma once
#include "message.h"

#include "augs/misc/value_meter.h"

namespace messages {
	struct health_event : message {
		enum class result_type {
			NONE,
			PERSONAL_ELECTRICITY_DESTRUCTION,
			DEATH,
			LOSS_OF_CONSCIOUSNESS
		} special_result = result_type::NONE;

		enum class target_type {
			INVALID,
			AIM,
			PERSONAL_ELECTRICITY,
			CONSCIOUSNESS,
			HEALTH
		} target = target_type::INVALID;

		vec2 point_of_impact;
		vec2 impact_velocity;

		float ratio_effective_to_maximum = 1.f;
		meter_value_type effective_amount = 0;
	};
}