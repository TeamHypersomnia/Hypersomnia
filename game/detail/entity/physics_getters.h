#pragma once

template<class entity_handle_type>
class physics_getters {
public:
	entity_handle_type get_owner_friction_field() const;
	entity_handle_type get_owner_body_entity() const;
};