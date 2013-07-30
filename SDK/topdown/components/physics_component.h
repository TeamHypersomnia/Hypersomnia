#pragma once
#include <Box2D\Box2D.h>

#include "../../../entity_system/entity_system.h"
#define PIXELS_TO_METERS 1.0/100
#define METERS_TO_PIXELS 100.0

namespace components {
	struct physics : public augmentations::entity_system::component {
		b2Body* body;
		physics(b2Body* body) : body(body) {}
	};
}
