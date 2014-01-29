#pragma once
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"


namespace components {
	struct damage : public augs::entity_system::component {
		float amount;
		augs::entity_system::entity_ptr sender;

		/* used to destroy bullets */
		augs::vec2<> starting_point;
		float max_distance;


		damage() : amount(0.f), sender(nullptr), max_distance(-1.f) {}
	};
}