#pragma once
#include "entity_system/processing_system.h"
#include "../components/physics_component.h"
#include "../components/movement_component.h"

using namespace augs;

class movement_system : public processing_system_templated<components::movement> {
public:
	using processing_system_templated::processing_system_templated;

	void set_movement_flags_from_input();
	void apply_movement_forces();
	void generate_movement_responses();
};