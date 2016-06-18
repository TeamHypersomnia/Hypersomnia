#pragma once
#include "game/processing_system_with_cosmos_reference.h"
#include "game/components/driver_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

class driver_system : public augs::processing_system_templated<components::driver, components::transform, components::physics> {
	bool change_car_ownership(entity_id driver, entity_id car, bool lost);
	bool release_car_ownership(entity_id driver);
	bool assign_car_ownership(entity_id driver, entity_id car);
public:
	using processing_system_templated::processing_system_templated;

	void assign_drivers_from_successful_trigger_hits();

	void release_drivers_due_to_requests();
	void release_drivers_due_to_ending_contact_with_wheel();
};