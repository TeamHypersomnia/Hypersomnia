#pragma once
#include "entity_system/entity.h"
#include "math/vec2.h"
#include <vector>
#include "transform_component.h"

class b2Fixture;
class b2Body;

namespace components {
	struct fixtures {
		struct fixture_state {
			b2Fixture* fixture;
			//augs::entity_id owner_friction_ground;
		};

		fixture_state* find_fixture(b2Fixture*);

		std::vector<fixture_state> list_of_fixtures;

		bool is_friction_ground = false;

		components::transform shape_offset;

		std::vector<std::vector <vec2>> convex_polys;

		b2Body* get_body();
		augs::entity_id get_body_entity();
	};
}