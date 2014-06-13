#pragma once
#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"

using namespace augs;
using namespace entity_system;

class script_system : public processing_system_templated<components::scriptable> {
public:
	void pass_events(world&, bool);
	void call_loop(world&, bool);

	void process_entities(world&);
	void process_events(world&);
	void substep(world&);
};