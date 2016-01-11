#pragma once
#include "entity_system/processing_system.h"
#include "../components/driver_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

class driver_system : public augs::processing_system_templated<components::driver, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void process_vehicle_ownership();
	void issue_commands_to_steered_vehicles();
};