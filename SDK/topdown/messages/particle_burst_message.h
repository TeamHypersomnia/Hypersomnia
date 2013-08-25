#pragma once
#include "message.h"
#include "math/vec2d.h"

namespace messages {
	struct particle_burst_message : public message {
		enum class burst_type {
			BULLET_IMPACT,
			WEAPON_SHOT
		} type;

		augmentations::vec2<> pos;
		float rotation;
	};
}