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
}