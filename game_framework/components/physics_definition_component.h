#pragma once
#include "../shared/physics_setup_helpers.h"

namespace components {
	struct physics_definition {
		bool dont_create_fixtures_and_body = false;
		bool preserve_definition_for_cloning = false;

		body_definition body;
		std::vector<fixture_definition> fixtures;
		std::vector<std::pair<fixture_definition, augs::entity_id>> fixtures_linked_to_other_bodies;

		fixture_definition& new_fixture(augs::entity_id attached_to_entity = augs::entity_id()) {
			if (attached_to_entity.alive()) {
				fixtures_linked_to_other_bodies.push_back(std::make_pair(fixture_definition(), attached_to_entity));
				return (*fixtures_linked_to_other_bodies.rbegin()).first;
			}
			else {
				fixtures.push_back(fixture_definition());
				return *fixtures.rbegin();
			}
		}
	};
}