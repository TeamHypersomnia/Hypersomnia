#include "fixtures_component.h"
#include <Box2D\Dynamics\b2Fixture.h>


namespace components {
	fixtures::fixture_state* fixtures::find_fixture(b2Fixture* f) {
		assert(list_of_fixtures.size() > 0);
		for (auto& ff : list_of_fixtures) {
			if (ff.fixture == f)
				return &ff;
		}

		return nullptr;
	}

	b2Body* fixtures::get_body() {
		assert(list_of_fixtures.size() > 0);
		return list_of_fixtures[0].fixture->GetBody();
	}

	augs::entity_id fixtures::get_body_entity() {
		assert(list_of_fixtures.size() > 0);
		return list_of_fixtures[0].fixture->GetBody()->GetUserData();
	}

	vec2 fixtures::get_aabb_size() {
		b2AABB aabb;
		aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
		aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

		for(auto& fixture : list_of_fixtures)
			aabb.Combine(aabb, fixture.fixture->GetAABB(0));

		return vec2(aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y);
	}
}