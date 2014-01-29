#pragma once
#include "entity_system/processing_system.h"
#include "../components/physics_component.h"
#include "../components/movement_component.h"

using namespace augmentations;
using namespace entity_system;

class movement_system : public processing_system_templated<components::movement, components::physics> {
public:
	void consume_events(world&) override;
	void substep(world&) override;
	void process_entities(world&) override;
};