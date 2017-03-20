#include "grenade_logic.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/components/physics_component.h"
#include "game/components/sprite_component.h"
#include "game/components/grenade_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/sound_existence_component.h"

#include "game/systems_stateless/sound_existence_system.h"

void release_or_throw_grenade(
	const logic_step step,
	const entity_handle grenade_entity,
	const entity_id thrower_id
) {
	auto& cosmos = step.cosm;
	const auto now = cosmos.get_timestamp();
	const auto delta = step.get_delta();
	const auto thrower = cosmos[thrower_id];
	const auto thrower_transform = thrower.get_logic_transform();
	//const auto thrower_orientation = vec2().set_from_degrees(thrower_transform.rotation);
	
	// LOG("throrot: %x", thrower_transform.rotation);

	auto& grenade = grenade_entity.get<components::grenade>();

	if (!grenade.when_released.was_set()) {
		grenade_entity.get<components::processing>().enable_in(processing_subjects::WITH_GRENADE);

		grenade.when_released = now;
		grenade.when_explodes.step = static_cast<unsigned>(now.step + (1 / delta.in_seconds() * 1.0));

		sound_effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = thrower;
		in.effect = assets::sound_buffer_id::GRENADE_UNPIN;

		sound_existence_system().create_sound_effect_entity(cosmos, in, thrower_transform, thrower).add_standard_components();
	}
	else {
		perform_transfer(
			cosmos [ item_slot_transfer_request_data{ grenade_entity, inventory_slot_id() }], 
			step
		);

		sound_effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = thrower;
		in.effect = assets::sound_buffer_id::GRENADE_THROW;

		sound_existence_system().create_sound_effect_entity(cosmos, in, thrower_transform, thrower).add_standard_components();

		// {
		// 	const auto spoon = cosmos[grenade.spoon];
		// 
		// 	auto& physics = spoon.get<components::physics>();
		// 	
		// 	spoon.set_logic_transform(thrower_transform);
		// 	spoon.add_standard_components();
		// 	
		// 	physics.apply_impulse(thrower_orientation.perpendicular_cw() * 1000 * physics.get_mass());
		// 
		// 	grenade.released_spoon = grenade.spoon;
		// 	grenade.spoon.unset();
		// }

		grenade_entity.get<components::sprite>().set(grenade.released_image_id);

		auto& physics = grenade_entity.get<components::physics>();
		physics.apply_impulse(vec2().set_from_degrees(grenade_entity.get_logic_transform().rotation) * 2000 * physics.get_mass());
		physics.set_bullet_body(true);
		physics.set_linear_damping(3.0f);

		auto& fixtures = grenade_entity.get<components::fixtures>();
		auto new_def = fixtures.get_data();
		new_def.colliders[0].restitution = 0.6;
		new_def.colliders[0].density = 10.f;

		const auto aabb = grenade_entity.get_aabb();
		const auto new_radius = 1.f;// std::min(aabb.w(), aabb.h()) / 16;// aabb.diagonal() / 2;
		new_def.colliders[0].shape.set(circle_shape{ new_radius });

		for (auto& c : new_def.colliders) {
			c.material = physical_material_type::GRENADE;
		}

		fixtures = new_def;
		//new_def.colliders[0].shape.. = 1.f;

		//auto new_def = physics.get_data();
		//new_def.bullet = true;
		//
		//physics = new_def;
	}
}