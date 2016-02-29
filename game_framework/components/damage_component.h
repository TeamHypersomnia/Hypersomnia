#pragma once
#include "math/vec2.h"

#include "entity_system/entity.h"

#include "misc/timer.h"


namespace components {
	struct damage {
		float amount = 12.f;
		
		augs::entity_id sender;
		bool destroy_upon_hit = true;

		/* used to destroy bullets */
		vec2 starting_point;

		bool constrain_lifetime = true;
		bool constrain_distance = false;

		float max_distance = 0.f;
		float max_lifetime_ms = 2000.f;
		
		float lifetime_ms = 0.f;
	};
}