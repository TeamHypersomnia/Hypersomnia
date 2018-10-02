#include "augs/templates/container_templates.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"
#include "physics_scripts.h"

void physics_world_cache::rechoose_owner_friction_body(const entity_handle entity) {
#if TODO_CARS
	if (const auto cache = find_rigid_body_cache(entity)) {
		auto& special_physics = entity.get_special_physics();
		auto& cosm = entity.get_cosmos();
		// purge of dead entities

		erase_if(special_physics.owner_friction_grounds, [&cosm](const auto subject) {
			return cosm[subject.target].dead();
		});

		auto feasible_grounds = special_physics.owner_friction_grounds;

		if (!feasible_grounds.empty()) {
			// cycle guard
			// remove friction grounds whom I do own myself

			erase_if(feasible_grounds, [entity, &cosm](const auto subject) {
				return ::are_connected_by_friction(cosm[subject.target], entity);
			});
		}

		const auto body = cache->body.get();

		if (!feasible_grounds.empty()) {
			std::stable_sort(feasible_grounds.begin(), feasible_grounds.end(), [&cosm](const auto a, const auto b) {
				return ::are_connected_by_friction(cosm[a.target], cosm[b.target]);
			});

			special_physics.owner_friction_ground = feasible_grounds[0].target;
			body->m_ownerFrictionGround = find_rigid_body_cache(cosm[special_physics.owner_friction_ground])->body;

			// make the new owner first in order in case it is later compared to the same ancestor-level parents

			for (auto& it : special_physics.owner_friction_grounds) {
				if (it.target == special_physics.owner_friction_ground) {
					std::swap(special_physics.owner_friction_grounds[0], it);
				}
			}

			/// consider friction grounds ONLY from the same ancestor line, and only the descendants

			/// if the current one is not found within contacting friction grounds,
			/// prioritize like this:
			/// firstly, the lowest descendant of the ancestor line of the lost friction ground
			/// descendant of any other tree with the biggest height, stable-sorted in order of entrance

		}
		else {
			special_physics.owner_friction_ground.unset();
			body->m_ownerFrictionGround = nullptr;
		}
	}
#else
	(void)entity;
#endif
}

void physics_world_cache::recurential_friction_handler(const logic_step step, b2Body* const body, b2Body* const friction_entity) {
	if (friction_entity == nullptr) {
		return;
	}

	/* TODO: ensure that m_ownerFrictionGround cannot point to a dead body */
	recurential_friction_handler(step, body, friction_entity->m_ownerFrictionGround);

	/*
		Start "fricting" positions from the deepest level
	*/

	const auto dt = static_cast<float>(step.get_delta().in_seconds());
	const auto fricted_pos = body->GetPosition() + dt* friction_entity->GetLinearVelocityFromWorldPoint(body->GetPosition());

	body->SetTransform(fricted_pos, body->GetAngle() + dt*friction_entity->GetAngularVelocity());

	//friction_entity.get_special_physics().measured_carried_mass += rigid_body.get_mass() + entity.get_special_physics().measured_carried_mass;
}
