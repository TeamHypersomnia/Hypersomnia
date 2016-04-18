#pragma once
#include "entity_system/entity_id.h"
#include "math/vec2.h"

namespace messages {
	struct physics_operation {
		augs::entity_id subject;
		vec2 apply_force;
		vec2 force_offset;

		bool reset_drop_timeout = false;
		float timeout_ms = 0.f;

		bool set_velocity = false;
		vec2 velocity;
	};
}