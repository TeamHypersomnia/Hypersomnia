#include "game/temporary_systems/physics_system.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "physics_scripts.h"

void physics_system::rechoose_owner_friction_body(const entity_handle entity) {
	auto& physics = entity.get<components::special_physics>();
	auto& cosmos = entity.get_cosmos();
	// purge of dead entities

	erase_remove(physics.owner_friction_grounds, [&cosmos](const entity_id subject) {
		return cosmos[subject].dead();
	});

	auto feasible_grounds = physics.owner_friction_grounds;

	if (!feasible_grounds.empty()) {
		// cycle guard
		// remove friction grounds whom I do own myself

		erase_remove(feasible_grounds, [this, entity, &cosmos](entity_id subject) {
			return are_connected_by_friction(cosmos[subject], entity);
		});
	}

	if (!feasible_grounds.empty()) {
		std::stable_sort(feasible_grounds.begin(), feasible_grounds.end(), [this, &cosmos](const entity_id a, const entity_id b) {
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

void physics_system::recurential_friction_handler(fixed_step& step, const entity_handle entity, const entity_handle friction_owner) {
	if (friction_owner.dead()) return;

	const float dt = static_cast<float>(step.get_delta().in_seconds());

	const auto& physics = entity.get<components::physics>();

	auto& friction_entity = friction_owner.get_owner_body();

	recurential_friction_handler(step, entity, friction_entity.get_owner_friction_ground());

	auto* const body = get_rigid_body_cache(entity).body;

	const auto friction_body = get_rigid_body_cache(friction_entity).body;
	const auto fricted_pos = body->GetPosition() + dt* friction_body->GetLinearVelocityFromWorldPoint(body->GetPosition());

	body->SetTransform(fricted_pos, body->GetAngle() + dt*friction_body->GetAngularVelocity());

	friction_entity.get<components::special_physics>().measured_carried_mass += physics.get_mass() + entity.get<components::special_physics>().measured_carried_mass;
}
