#pragma once
#include "message.h"
#include "math/vec2.h"

namespace messages {
	struct damage_message : public message {
		float amount;
		augs::entity_id inflictor;
		vec2 impact_velocity;
		vec2 point_of_impact;
	};
}