#pragma once

class cosmos;
class step_state;

class driver_system {
	bool change_car_ownership(cosmos& cosmos, entity_handle driver, entity_handle car, bool lost);
	bool release_car_ownership(cosmos& cosmos, entity_handle driver);
	bool assign_car_ownership(cosmos& cosmos, entity_handle driver, entity_handle car);
public:
	void assign_drivers_from_successful_trigger_hits(cosmos& cosmos, step_state& step);

	void release_drivers_due_to_requests(cosmos& cosmos, step_state& step);
	void release_drivers_due_to_ending_contact_with_wheel(cosmos& cosmos, step_state& step);
};