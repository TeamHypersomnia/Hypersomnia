#pragma once
#include <Box2D\Box2D.h>

#include "../../../entity_system/entity_system.h"

namespace components {
	struct physics : public augmentations::entity_system::component {
		b2Body* body;
		physics() : body(nullptr) {}
	};
}
