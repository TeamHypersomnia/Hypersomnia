#pragma once
#include "augs/templates/propagate_const.h"
#include "game/container_sizes.h"

class b2Body;
class b2Fixture;
class cosmos;

struct rigid_body_cache {
	static constexpr bool is_cache = true;

	augs::propagate_const<b2Body*> body = nullptr;

	void clear(cosmos&, physics_world_cache&);

	bool is_constructed() const {
		return body.get() != nullptr;
	}
};

struct colliders_cache {
	static constexpr bool is_cache = true;

	std::vector<augs::propagate_const<b2Fixture*>> constructed_fixtures;

	colliders_connection connection;

	void clear(physics_world_cache&);
	void clear_fields();

	bool is_constructed() const {
		return connection.owner.is_set();
	}
};
