#pragma once
#include "message.h"
#include "math/vec2d.h"

namespace messages {
	struct damage_message : public message {
		float amount;
		augs::vec2<> impact_velocity;
	};
}