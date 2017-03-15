#include "grenade_logic.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"

void release_or_throw_grenade(
	const logic_step step,
	const entity_handle grenade,
	const entity_id thrower_id
) {
	auto& cosmos = step.cosm;
	const auto thrower = cosmos[thrower_id];

	perform_transfer(
		cosmos [ item_slot_transfer_request_data{ grenade, inventory_slot_id() }], 
		step
	);

	auto& physics = grenade.get<components::physics>();
	physics.apply_impulse(vec2().set_from_degrees(thrower.get_logic_transform().rotation) * 2000 * physics.get_mass());
	physics.set_bullet_body(true);
	physics.set_linear_damping(3.0f);

	auto& fixtures = grenade.get<components::fixtures>();
	auto new_def = fixtures.get_data();
	new_def.colliders[0].restitution = 1.0f;

	const auto aabb = grenade.get_aabb();
	const auto new_radius = 7.f;// std::min(aabb.w(), aabb.h()) / 4;// aabb.diagonal() / 2;
	new_def.colliders[0].shape.set(circle_shape{ new_radius });

	fixtures = new_def;
	//new_def.colliders[0].shape.. = 1.f;

	//auto new_def = physics.get_data();
	//new_def.bullet = true;
	//
	//physics = new_def;
}