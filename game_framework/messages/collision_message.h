#pragma once
#include "message.h"
#include "math/vec2.h"

namespace messages {
	struct collision_message : public message {
		augs::entity_system::entity_id collider;
		augs::vec2<> collider_impact_velocity, subject_impact_velocity, point;

		bool sensor_end_contact = false;
		collision_message() { send_to_scripts = true; }
	};
}