#pragma once
#include <Box2D\Box2D.h>

struct sprite;

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace topdown {
	extern void create_physics_component(augmentations::entity_system::entity& subject, b2World&, b2Filter filter, b2BodyType = b2_dynamicBody);
}