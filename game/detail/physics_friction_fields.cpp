#include "game/temporary_systems/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/transcendental/cosmos.h"
#include "physics_scripts.h"

void physics_system::rechoose_owner_friction_body(entity_handle entity) {
	auto& physics = entity.get<components::special_physics>();
	auto& cosmos = entity.get_cosmos();
	// purge of dead entities

	physics.owner_friction_grounds.erase(std::remove_if(physics.owner_friction_grounds.begin(), physics.owner_friction_grounds.end(), [&cosmos](entity_id subject) {
		return cosmos[subject].dead();
	}), physics.owner_friction_grounds.end());

	auto feasible_grounds = physics.owner_friction_grounds;

	if (!feasible_grounds.empty()) {
		// cycle guard
		// remove friction grounds whom do I own myself

		feasible_grounds.erase(std::remove_if(feasible_grounds.begin(), feasible_grounds.end(), [this, entity, &cosmos](entity_id subject) {
			return are_connected_by_friction(cosmos[subject], entity);
		}), feasible_grounds.end());
	}

	if (!feasible_grounds.empty()) {
		std::stable_sort(feasible_grounds.begin(), feasible_grounds.end(), [this, &cosmos](entity_id a, entity_id b) {
			return are_connected_by_friction(cosmos[a], cosmos[b]);
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

void physics_system::recurential_friction_handler(fixed_step& step, entity_handle entity, entity_handle friction_owner) {
	if (friction_owner.dead()) return;

	float dt = static_cast<float>(step.get_delta().in_seconds());

	auto& physics = entity.get<components::physics>();

	auto& friction_physics = friction_owner.get<components::fixtures>();
	auto& friction_entity = friction_physics.get_owner_body();

	recurential_friction_handler(step, entity, friction_entity.get_owner_friction_ground());

	auto* body = get_rigid_body_cache(entity).body;

	auto friction_body = get_rigid_body_cache(friction_entity).body;
	auto fricted_pos = body->GetPosition() + dt* friction_body->GetLinearVelocityFromWorldPoint(body->GetPosition());

	body->SetTransform(fricted_pos, body->GetAngle() + dt*friction_body->GetAngularVelocity());

	friction_entity.get<components::special_physics>().measured_carried_mass += physics.get_mass() + entity.get<components::special_physics>().measured_carried_mass;
}
