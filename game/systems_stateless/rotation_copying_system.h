#pragma once
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
#include "game/transcendental/step_declaration.h"

class rotation_copying_system {
	float resolve_rotation_copying_value(const_entity_handle) const;
public:

	void update_rotations(cosmos& cosmos) const;
};