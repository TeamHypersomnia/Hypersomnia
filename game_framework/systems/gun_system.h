#pragma once
#include <random>
#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/gun_component.h"

using namespace augs;

class physics_system;

class gun_system : public processing_system_templated<components::transform, components::gun> {
public:
	using processing_system_templated::processing_system_templated;
	
	void consume_events();
	void process_entities();
};