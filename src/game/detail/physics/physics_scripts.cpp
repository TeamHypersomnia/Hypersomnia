#include "physics_scripts.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/sentience_component.h"
#include "game/components/movement_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/inventory/inventory_slot_handle.h"

void resolve_dampings_of_body(
	const entity_handle it,
	const bool is_sprint_effective
) {
	auto& cosmos = it.get_cosmos();
	const auto rigid_body = it.get<components::rigid_body>();

	auto considered_damping = 0.f;

	if (it.get<components::processing>().is_in(processing_subjects::WITH_MOVEMENT)) {
		const auto& movement = it.get<components::movement>();

		auto* const sentience = it.find<components::sentience>();

		const bool is_inert = movement.make_inert_for_ms > 0.f;

		if (is_inert) {
			considered_damping = 2;
		}
		else {
			considered_damping = movement.standard_linear_damping;
		}

		const auto requested_by_input = movement.get_force_requested_by_input();

		if (requested_by_input.non_zero()) {
			if (is_sprint_effective) {
				if (!is_inert) {
					considered_damping /= 4;
				}
			}
		}

		rigid_body.set_linear_damping(considered_damping);

		/* the player feels less like a physical projectile if we brake per-axis */
		if (movement.enable_braking_damping && !(movement.make_inert_for_ms > 0.f)) {
			rigid_body.set_linear_damping_vec(
				vec2(
					requested_by_input.x_non_zero() ? 0.f : movement.braking_damping,
					requested_by_input.y_non_zero() ? 0.f : movement.braking_damping
				)
			);
		}
		else {
			rigid_body.set_linear_damping_vec(vec2(0, 0));
		}
	}
	else {
		rigid_body.set_linear_damping_vec(components::rigid_body().linear_damping_vec);
		rigid_body.set_linear_damping(components::rigid_body().linear_damping);
		rigid_body.set_angular_damping(components::rigid_body().angular_damping);
	}
}

void resolve_density_of_associated_fixtures(const entity_handle id) {
	auto& cosmos = id.get_cosmos();

	{
		if (const auto rigid_body = id.find<components::rigid_body>()) {
			const auto entities = rigid_body.get_fixture_entities();

			for (const auto f : entities) {
				if (f != id.get_id()) {
					resolve_density_of_associated_fixtures(cosmos[f]);
				}
			}
		}
	}

	const auto fixtures = id.get<components::fixtures>();

	float density_multiplier = 1.f;

	const auto* const item = id.find<components::item>();

	if (item != nullptr && cosmos[item->current_slot].alive() && cosmos[item->current_slot].is_physically_connected_until()) {
		density_multiplier *= cosmos[item->current_slot].calculate_density_multiplier_due_to_being_attached();
	}

	const auto owner_body = id.get_owner_body();
	const auto* const driver = owner_body.find<components::driver>();

	if (driver) {
		if (cosmos[driver->owned_vehicle].alive()) {
			density_multiplier *= driver->density_multiplier_while_driving;
		}
	}

	fixtures.set_density_multiplier(density_multiplier);
}

bool are_connected_by_friction(
	const const_entity_handle child, 
	const const_entity_handle parent
) {
	const auto& cosmos = child.get_cosmos();

	bool matched_ancestor = false;

	const auto owner_body_of_parent = parent.get_owner_body();
	const auto owner_body_of_child = child.get_owner_body();

	if(owner_body_of_child.alive()) {
		entity_id childs_ancestor_entity = owner_body_of_child.get_owner_friction_ground();

		while (cosmos[childs_ancestor_entity].alive()) {
			if (childs_ancestor_entity == owner_body_of_parent) {
				matched_ancestor = true;
				break;
			}

			childs_ancestor_entity = cosmos[childs_ancestor_entity].get_owner_friction_ground();
		}
	}

	return matched_ancestor;
}