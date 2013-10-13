#pragma once
#include <random>
#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/gun_component.h"

using namespace augmentations;
using namespace entity_system;

class physics_system;

class gun_system : public processing_system_templated<components::transform, components::gun> {
	std::random_device device;
	std::mt19937 generator;
public:
	gun_system();
	void process_entities(world&) override;
};