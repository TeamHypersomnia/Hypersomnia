#pragma once
#include "augs/misc/children_vector_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

class relational_cache {
	augs::children_vector_tracker<unversioned_entity_id, 1> fixtures_of_bodies;
	augs::children_vector_tracker<unversioned_entity_id, 2> joints_of_bodies;

	template <class F>
	void for_each_tracker(F f) {
		f(fixtures_of_bodies);
		f(joints_of_bodies);
	}

public:
	void handle_deletion_of_potential_parent(const entity_id);

	void reserve_caches_for_entities(const size_t n);
	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);

	void set_fixtures_parent(
		const entity_id fixtures, 
		const entity_id new_owner
	) {
		fixtures_of_bodies.set_parent(fixtures, new_owner);
	}

	const auto& get_fixtures_of_bodies() const {
		return fixtures_of_bodies;
	}

	const auto& get_joints_of_bodies() const {
		return joints_of_bodies;
	}
};