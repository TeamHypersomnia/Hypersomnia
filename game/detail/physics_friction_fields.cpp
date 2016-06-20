#include "game/systems/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos.h"

void physics_system::rechoose_owner_friction_body(entity_id entity) {
	auto& physics = entity.get<components::physics>();

	// purge of dead entities

	physics.owner_friction_grounds.erase(std::remove_if(physics.owner_friction_grounds.begin(), physics.owner_friction_grounds.end(), [entity](entity_id subject) {
		return subject.dead();
	}), physics.owner_friction_grounds.end());

	auto feasible_grounds = physics.owner_friction_grounds;

	if (!feasible_grounds.empty()) {
		// cycle guard
		// remove friction grounds whom do I own myself

		feasible_grounds.erase(std::remove_if(feasible_grounds.begin(), feasible_grounds.end(), [this, entity](entity_id subject) {
			return are_connected_by_friction(subject, entity);
		}), feasible_grounds.end());
	}

	if (!feasible_grounds.empty()) {
		std::stable_sort(feasible_grounds.begin(), feasible_grounds.end(), [this](entity_id a, entity_id b) {
			return are_connected_by_friction(a, b);
		});

		physics.owner_friction_ground = feasible_grounds[0];

		// make the new owner first in order in case it is later compared to the same ancestor-level parents

		for (auto& it = physics.owner_friction_grounds.begin(); it != physics.owner_friction_grounds.end(); ++it) {
			if (*it == physics.owner_friction_ground) {
				std::swap(physics.owner_friction_grounds[0], *it);
			}
		}

		/// consider friction grounds ONLY from the same ancestor line, and only the descendants

		/// if the current one is not found within contacting friction grounds,
		/// prioritize like this:
		/// firstly, the lowest descendant of the ancestor line of the lost friction ground
		/// descendant of any other tree with the biggest height, stable-sorted in order of entrance

	}
	else {
		physics.owner_friction_ground.unset();
	}
}

void physics_system::recurential_friction_handler(entity_id entity, entity_id friction_owner) {
	if (friction_owner.dead()) return;

	auto& physics = entity.get<components::physics>();

	auto& friction_physics = friction_owner.get<components::fixtures>();
	auto& friction_entity = friction_physics.get_body_entity();

	recurential_friction_handler(entity, friction_entity.get<components::physics>().get_owner_friction_ground());

	auto* body = physics.black_detail.body;

	auto friction_body = friction_physics.get_body();
	auto fricted_pos = body->GetPosition() + parent_cosmos.delta.in_seconds() * friction_body->GetLinearVelocityFromWorldPoint(body->GetPosition());

	body->SetTransform(fricted_pos, body->GetAngle() + parent_cosmos.delta.in_seconds()*friction_body->GetAngularVelocity());

	friction_entity.get<components::physics>().measured_carried_mass += physics.get_mass() + physics.measured_carried_mass;
}

bool physics_system::are_connected_by_friction(entity_id child, entity_id parent) {
	if (is_entity_physical(child) && is_entity_physical(parent)) {
		bool matched_ancestor = false;

		entity_id parent_body_entity = get_owner_body_entity(parent);
		entity_id childs_ancestor_entity = get_owner_body_entity(child).get<components::physics>().get_owner_friction_ground();

		while (childs_ancestor_entity.alive()) {
			if (childs_ancestor_entity == parent_body_entity) {
				matched_ancestor = true;
				break;
			}

			childs_ancestor_entity = childs_ancestor_entity.get<components::physics>().get_owner_friction_ground();
		}

		if (matched_ancestor)
			return true;
	}

	return false;
}
