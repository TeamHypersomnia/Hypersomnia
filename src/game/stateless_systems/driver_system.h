#pragma once
#include "game/cosmos/entity_handle_declaration.h"

class cosmos;
#include "game/cosmos/step_declaration.h"

class driver_system {
	bool change_car_ownership(entity_handle driver, entity_handle car);
public:
	bool release_car_ownership(entity_handle driver);
	bool assign_car_ownership(entity_handle driver, entity_handle car);

	void assign_drivers_who_touch_wheels(const logic_step step);

	void release_drivers_due_to_requests(const logic_step step);
	void release_drivers_due_to_ending_contact_with_wheel(const logic_step step);
};