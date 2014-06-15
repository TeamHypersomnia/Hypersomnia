#pragma once
#include "message.h"
#include "math/vec2d.h"

namespace messages {
	struct collision_message : public message {
		augs::entity_system::entity* collider;
		augs::vec2<> impact_velocity, point;

		bool sensor_end_contact;
		collision_message() : sensor_end_contact(false) { send_to_scripts = true;  }
	};
}