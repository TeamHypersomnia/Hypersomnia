#pragma once
#include "game/transcendental/entity_handle_declaration.h"

template<bool is_const, class entity_handle_type>
class physics_mixin {
public:
	entity_handle_type get_owner_friction_ground() const;
};