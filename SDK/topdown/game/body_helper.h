#pragma once
#include "entity_system/entity_system.h"
#include <Box2D\Box2D.h>

struct sprite;
namespace topdown {
	extern void create_physics_component(augmentations::entity_system::entity& subject, b2World&, b2BodyType = b2_dynamicBody);
}