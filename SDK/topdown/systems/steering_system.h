#pragma once
#include "entity_system/processing_system.h"

#include "../components/steering_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"
#include "../components/visibility_component.h"

using namespace augmentations;
using namespace entity_system;

class physics_system;

class steering_system : public processing_system_templated<components::transform, components::physics, components::steering> {
public:
	void substep(world&) override;
	void process_entities(world&) override;
	void process_events(world&) override;
};