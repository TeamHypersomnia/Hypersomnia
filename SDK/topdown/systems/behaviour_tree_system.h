#pragma once
#include "entity_system/processing_system.h"

#include "../components/behaviour_tree_component.h"

using namespace augmentations;
using namespace entity_system;

class physics_system;

class behaviour_tree_system : public processing_system_templated<components::behaviour_tree> {
public:
	void substep(world&) override;
	//void process_entities(world&) override;
};