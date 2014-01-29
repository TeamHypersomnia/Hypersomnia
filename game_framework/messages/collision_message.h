#pragma once
#include "message.h"
#include "math/vec2d.h"

namespace messages {
	struct collision_message : public message {
		augmentations::entity_system::entity* collider;
		augmentations::vec2<> impact_velocity, point;

		bool sensor_end_contact;
		collision_message() : sensor_end_contact(false) {}
	};
}