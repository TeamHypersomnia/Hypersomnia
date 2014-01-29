#pragma once
#include "entity_system/processing_system.h"
#include "../components/children_component.h"

using namespace augs;
using namespace entity_system;

class destroy_system : public processing_system_templated<components::children> {
public:
	void add(entity*) override {}
	void remove(entity*) override {}

	void consume_events(world&) override;
};