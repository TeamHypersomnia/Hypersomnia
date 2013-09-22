#pragma once
#include <Box2D\Box2D.h>
#include <string>

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace topdown {
	extern b2World* current_b2world;

	extern void create_physics_component_str(augmentations::entity_system::entity& subject, b2Filter filter, const std::string& body_type = "dynamic");
	extern void create_physics_component    (augmentations::entity_system::entity& subject, b2Filter filter, b2BodyType = b2_dynamicBody);
}