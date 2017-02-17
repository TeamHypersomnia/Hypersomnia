#pragma once
#include "math/vec2.h"
#include <Box2D\Dynamics\b2WorldCallbacks.h>

#include "game/messages/visibility_information.h"

namespace messages {
	struct exploding_ring {
		float outer_radius_start_value = 0.f;
		float outer_radius_end_value = 0.f;
		float inner_radius_start_value = 0.f;
		float inner_radius_end_value = 0.f;

		float maximum_duration_seconds = 0.f;
		float time_of_occurence = 0.f;

		vec2 center;
		
		messages::visibility_information_response visibility;
		rgba color;
	};
}