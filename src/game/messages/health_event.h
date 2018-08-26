#pragma once
#include "game/messages/message.h"
#include "game/detail/damage_origin.h"

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
			PERSONAL_ELECTRICITY,
			CONSCIOUSNESS,
			HEALTH
		} target = target_type::INVALID;

		vec2 point_of_impact;
		vec2 impact_velocity;

		damage_origin origin;

		float ratio_effective_to_maximum = 1.f;
		meter_value_type effective_amount = 0;

		bool was_conscious = true;
	};
}