#pragma once
#include "message.h"
#include "augs/math/vec2.h"

namespace messages {
	struct damage_message : public message {
		bool inflictor_destructed = false;
		float amount = 0.f;
		entity_id inflictor;
		vec2 impact_velocity;
		vec2 point_of_impact;
		augs::trivial_pair<size_t, size_t> subject_collider_and_convex_indices = std::make_pair(0u, 0u);
	};
}