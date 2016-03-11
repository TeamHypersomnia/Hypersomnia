#pragma once
#include "../detail/physics_setup_helpers.h"

namespace components {
	struct physics_definition {
		bool dont_create_fixtures_and_body = false;
		bool preserve_definition_for_cloning = true;

		body_definition body;
		std::vector<fixture_definition> fixtures;
		augs::entity_id attach_fixtures_to_entity;

		components::transform offset_created_shapes;

		fixture_definition& new_fixture(augs::entity_id attached_to_entity = augs::entity_id()) {
			attach_fixtures_to_entity = attached_to_entity;
			fixtures.push_back(fixture_definition());
			return *fixtures.rbegin(); 
		}
	};
}