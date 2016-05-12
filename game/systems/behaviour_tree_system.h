#pragma once
#include "entity_system/processing_system.h"
#include "../components/behaviour_tree_component.h"

using namespace augs;

class physics_system;

class behaviour_tree_system : public processing_system_templated<components::behaviour_tree> {
public:
	using processing_system_templated::processing_system_templated;

	void evaluate_trees();
};