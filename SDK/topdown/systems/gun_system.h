#pragma once
#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/gun_component.h"
#include <random>

using namespace augmentations;
using namespace entity_system;

class physics_system;
class gun_system : public processing_system_templated<components::transform, components::gun> {
	physics_system& physics;
	std::random_device device;
	std::mt19937 generator;
public:
	gun_system(physics_system&);
	void process_entities(world&) override;
};