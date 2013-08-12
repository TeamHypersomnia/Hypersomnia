#pragma once
#include "entity_system/entity.h"
#include "math/vec2d.h"

namespace components {
	struct chase : public augmentations::entity_system::component {
		augmentations::entity_system::entity* target;
		augmentations::vec2<float> offset, previous;
		bool relative;

		chase(augmentations::entity_system::entity* target, bool relative = false, augmentations::vec2<float> offset = augmentations::vec2<float>()) 
			: target(target), offset(offset), relative(relative) {}
	};
}