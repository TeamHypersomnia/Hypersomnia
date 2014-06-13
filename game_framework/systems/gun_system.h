#pragma once
#include <random>
#include "entity_system/processing_system.h"

#include "../components/transform_component.h"
#include "../components/gun_component.h"

using namespace augs;
using namespace entity_system;

class physics_system;

class gun_system : public processing_system_templated<components::transform, components::gun> {
public:
	void add(entity*) override;
	void remove(entity*) override;

	void consume_events(world&);
	void process_entities(world&);
};