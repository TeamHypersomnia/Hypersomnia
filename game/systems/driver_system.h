#pragma once
#include "entity_system/processing_system.h"
#include "../components/driver_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

class driver_system : public augs::processing_system_templated<components::driver, components::transform, components::physics> {
	bool change_car_ownership(augs::entity_id driver, augs::entity_id car, bool lost);
	bool release_car_ownership(augs::entity_id driver);
	bool assign_car_ownership(augs::entity_id driver, augs::entity_id car);
public:
	using processing_system_templated::processing_system_templated;

	void assign_drivers_from_successful_trigger_hits();

	void release_drivers_due_to_requests();
	void release_drivers_due_to_ending_contact_with_wheel();
};