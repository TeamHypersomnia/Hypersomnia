#pragma once
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
class fixed_step;

class rotation_copying_system {
	void resolve_rotation_copying_value(entity_handle) const;
public:

	void update_physical_motors(cosmos& cosmos) const;
	void update_rotations(cosmos& cosmos) const;
};