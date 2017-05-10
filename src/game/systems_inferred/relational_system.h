#pragma once
#include "augs/misc/parent_child_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/fixtures_component.h"

class relational_system {
public:
	friend class cosmos;
	friend class physics_system;
	friend void component_synchronizer<false, components::fixtures>::set_owner_body(const entity_id) const;

	void reserve_caches_for_entities(const size_t n);
	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);

	void handle_deletion_of_potential_parent(const entity_id);

	augs::parent_child_tracker<unversioned_entity_id, 1> fixtures_of_bodies;
	augs::parent_child_tracker<unversioned_entity_id, 2> joints_of_bodies;

	template <class F>
	void for_each_tracker(F f) {
		f(fixtures_of_bodies);
		f(joints_of_bodies);
	}
};