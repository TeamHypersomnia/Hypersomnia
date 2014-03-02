#pragma once
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

#include "misc/timer.h"


namespace components {
	struct damage : public augs::entity_system::component {
		float amount;
		augs::entity_system::entity_ptr sender;

		/* used to destroy bullets */
		augs::vec2<> starting_point;
		float max_distance;
		float max_lifetime_ms;
		augs::misc::timer lifetime;

		bool destroy_upon_hit;

		damage() : amount(0.f), sender(nullptr), max_distance(-1.f), max_lifetime_ms(-1.f), destroy_upon_hit(true) {}
	};
}