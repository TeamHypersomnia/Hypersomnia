#pragma once
#include "game/detail/physics_setup_helpers.h"

namespace components {
	struct physics_definition {
		bool create_fixtures_and_body = true;

		body_definition body;
		std::vector<fixture_definition> fixtures;
		entity_id attach_fixtures_to_entity;

		fixture_definition& new_fixture(entity_id attached_to_entity = entity_id()) {
			attach_fixtures_to_entity = attached_to_entity;
			fixtures.push_back(fixture_definition());
			return *fixtures.rbegin(); 
		}
	};
}