#pragma once
#include <Box2D\Box2D.h>

#include "entity_system/component.h"
#define METERS_TO_PIXELS 50.0
#define PIXELS_TO_METERS 1.0/METERS_TO_PIXELS

namespace components {
	struct physics : public augmentations::entity_system::component {
		b2Body* body;
		physics(b2Body* body) : body(body) {}
	};
}
