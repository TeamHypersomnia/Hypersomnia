#pragma once
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
class fixed_step;

class driver_system {
	bool change_car_ownership(entity_handle driver, entity_handle car, bool lost);
	bool release_car_ownership(entity_handle driver);
	bool assign_car_ownership(entity_handle driver, entity_handle car);
public:
	void assign_drivers_from_successful_trigger_hits(fixed_step& step);

	void release_drivers_due_to_requests(fixed_step& step);
	void release_drivers_due_to_ending_contact_with_wheel(fixed_step& step);
};