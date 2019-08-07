#pragma once
#include "augs/templates/propagate_const.h"
#include "game/container_sizes.h"

class b2Body;
class b2Fixture;
class cosmos;

struct rigid_body_cache {
	augs::propagate_const<b2Body*> body = nullptr;

	void clear(cosmos&, physics_world_cache&);

	bool is_constructed() const {
		return body.get() != nullptr;
	}
};

struct colliders_cache {
	augs::constant_size_vector<
		augs::propagate_const<b2Fixture*>, 
		POLY_VERTEX_COUNT
	> constructed_fixtures;

	colliders_connection connection;

	void clear(physics_world_cache&);

	bool is_constructed() const {
		return constructed_fixtures.size() > 0;
	}
};
