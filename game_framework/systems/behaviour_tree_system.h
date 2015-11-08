#pragma once
#include "entity_system/processing_system.h"

#include "../components/behaviour_tree_component.h"

using namespace augs;


class physics_system;

class behaviour_tree_system : public processing_system_templated<components::behaviour_tree> {
public:
	void substep(world&);
	void process_entities(world&);
};