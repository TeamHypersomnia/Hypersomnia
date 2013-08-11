#pragma once
#include "entity_system/entity.h"

namespace components {
	struct chase : public augmentations::entity_system::component {
		augmentations::entity_system::entity* target;

		chase(augmentations::entity_system::entity* target) : target(target) {}
	};
}