#pragma once

class cosmos;
class step_state;

class rotation_copying_system {
	void resolve_rotation_copying_value(cosmos& cosmos, entity_handle rotation_copying);
public:

	void update_physical_motors(cosmos& cosmos);
	void update_rotations(cosmos& cosmos);
};