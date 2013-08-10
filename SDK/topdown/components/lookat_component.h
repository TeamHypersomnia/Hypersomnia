#pragma once
#include "entity_system/entity.h"

namespace components {
	struct lookat : public augmentations::entity_system::component {
		augmentations::entity_system::entity* target;

		lookat(augmentations::entity_system::entity* target) : target(target) {}
	};
}