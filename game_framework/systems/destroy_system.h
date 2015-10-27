#pragma once
#include "entity_system/processing_system.h"
#include "../components/children_component.h"

using namespace augs;
using namespace entity_system;

class destroy_system : public processing_system_templated<components::children> {
public:
	void add(entity_id) override {}
	void remove(entity_id) override {}

	void consume_events(world&);
};