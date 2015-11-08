#pragma once
#include "math/vec2.h"
#include "entity_system/component.h"
#include "entity_system/entity.h"

#include "misc/timer.h"


namespace components {
	struct damage : public augs::entity_system::component {
		float amount;
		augs::entity_system::entity_id sender;

		/* used to destroy bullets */
		augs::vec2<> starting_point;
		float max_distance;
		float max_lifetime_ms;
		augs::misc::timer lifetime;

		bool destroy_upon_hit;

		damage() : amount(0.f), max_distance(-1.f), max_lifetime_ms(-1.f), destroy_upon_hit(true) {}
	};
}