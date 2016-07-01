#pragma once
#include "game/entity_handle_declaration.h"

template<bool is_const>
class physics_getters {
	typedef basic_entity_handle<is_const> entity_handle_type;
public:
	entity_handle_type get_owner_friction_field() const;
	entity_handle_type get_owner_body_entity() const;
};