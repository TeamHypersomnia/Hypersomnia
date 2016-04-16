#pragma once
#include "message.h"
#include "math/vec2.h"

namespace messages {
	struct collision_message : public message {
		augs::entity_id collider;
		vec2 collider_impact_velocity, subject_impact_velocity, point;

		enum class event_type {
			BEGIN_CONTACT,
			PRE_SOLVE,
			END_CONTACT
		} type;
	};
}