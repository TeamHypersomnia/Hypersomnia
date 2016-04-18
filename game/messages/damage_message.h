#pragma once
#include "message.h"
#include "math/vec2.h"

namespace messages {
	struct damage_message : public message {
		float amount;
		vec2 impact_velocity;
		vec2 point_of_impact;
	};
}