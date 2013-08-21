#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace components {
	struct damage : public augmentations::entity_system::component {
		float amount;
		augmentations::vec2<> starting_point;
		float max_distance;

		augmentations::entity_system::entity* sender;

		damage() : amount(0.f), sender(nullptr), max_distance(-1.f) {}
	};
}