#pragma once
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"


namespace components {
	struct damage : public augmentations::entity_system::component {
		float amount;
		augmentations::vec2<> starting_point;
		float max_distance;

		augmentations::entity_system::entity_ptr sender;

		damage() : amount(0.f), sender(nullptr), max_distance(-1.f) {}
	};
}