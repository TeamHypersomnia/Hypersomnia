#pragma once
#include "entity_system/component.h"

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace components {
	struct damage : public augmentations::entity_system::component {
		float amount;
		augmentations::entity_system::entity* sender;

	};
}