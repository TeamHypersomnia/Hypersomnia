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

		adverse_element_type source_adversity = adverse_element_type::FORCE;
		value_meter::damage_result damage;

		transformr head_transform;
		bool was_dead = false;
		bool was_conscious = true;
		bool is_remainder_after_shield_destruction = false;
		bool processed_special = false;
		bool play_headshot_sound = false;

		static auto request_death(
			const entity_id of_whom,
			const vec2 direction,
			const vec2 point_of_impact,
			const damage_origin& origin
		) {
			health_event output;
			output.subject = of_whom;
			output.point_of_impact = point_of_impact;
			output.impact_velocity = direction;
			output.special_result = result_type::DEATH;
			output.origin = origin;
			output.target = target_type::HEALTH;
			return output;
		}
	};
}