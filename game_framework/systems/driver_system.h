#pragma once
#include "entity_system/processing_system.h"
#include "../components/driver_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

#include "../messages/car_ownership_change_message.h"

class driver_system : public augs::processing_system_templated<components::driver, components::transform, components::physics> {
	void affect_drivers_due_to_car_ownership_changes(messages::car_ownership_change_message&);

public:
	using processing_system_templated::processing_system_templated;

	void assign_drivers_from_triggers();

	void release_drivers_due_to_requests();
	void release_drivers_due_to_distance();

	void delegate_movement_intents_from_drivers_to_steering_intents_of_owned_vehicles();

	void apply_forces_towards_wheels();
};