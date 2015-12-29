#pragma once
#include "entity_system/processing_system.h"
#include "../components/physics_component.h"
#include "../components/movement_component.h"

using namespace augs;

class movement_system : public processing_system_templated<components::movement, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_events();
	void substep();
	void process_entities();
};