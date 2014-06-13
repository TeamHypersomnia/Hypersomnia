#pragma once
#include "entity_system/processing_system.h"

#include "../components/damage_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

using namespace augs;
using namespace entity_system;

class damage_system : public processing_system_templated<components::damage, components::transform, components::physics> {
public:
	void process_events(world&);
	void process_entities(world&);
};