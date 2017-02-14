#pragma once
#include "math/vec2.h"
#include <Box2D\Dynamics\b2WorldCallbacks.h>

#include "game/messages/visibility_information.h"

namespace messages {
	struct exploding_ring {
		float min_radius = 0.f;
		float max_radius = 0.f;

		vec2 center;
		messages::visibility_information_response visibility;
		rgba color;
	};
}