#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sprite_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/grenade_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/sound_existence_component.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/all_inferred_state_component.h"

#include "game/systems_stateless/sound_existence_system.h"

void release_or_throw_grenade(
	const logic_step step,
	const entity_handle grenade_entity,
	const entity_id thrower_id,
	bool is_pressed_flag
) {
	auto& cosmos = step.cosm;
	const auto now = cosmos.get_timestamp();
	const auto delta = step.get_delta();
	const auto thrower = cosmos[thrower_id];
	const auto thrower_transform = thrower.get_logic_transform();
	//const auto thrower_orientation = vec2().set_from_degrees(thrower_transform.rotation);
	
	// LOG("throrot: %x", thrower_transform.rotation);

	auto& grenade = grenade_entity.get<components::grenade>();

	if (is_pressed_flag && !grenade.when_released.was_set()) {
		grenade_entity.get<components::processing>().enable_in(processing_subjects::WITH_GRENADE);

		grenade.when_released = now;
		grenade.when_explodes.step = static_cast<unsigned>(now.step + (1 / delta.in_seconds() * 1.0));

		sound_effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = thrower;
		in.effect.id = assets::sound_buffer_id::GRENADE_UNPIN;

		in.create_sound_effect_entity(step, thrower_transform, thrower).add_standard_components(step);
	}
	else if(!is_pressed_flag && grenade.when_released.was_set()) {
		perform_transfer(
			item_slot_transfer_request{ grenade_entity, inventory_slot_id(), -1, false, 0.f }, 
			step
		);
		
		sound_effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = thrower;
		in.effect.id = assets::sound_buffer_id::GRENADE_THROW;

		in.create_sound_effect_entity(step, thrower_transform, thrower).add_standard_components(step);

		// {
		// 	const auto spoon = cosmos[grenade.spoon];
		// 
		// 	auto& rigid_body = spoon.get<components::rigid_body>();
		// 	
		// 	spoon.set_logic_transform(step, thrower_transform);
		// 	spoon.add_standard_components(step);
		// 	
		// 	rigid_body.apply_impulse(thrower_orientation.perpendicular_cw() * 1000 * rigid_body.get_mass());
		// 
		// 	grenade.released_spoon = grenade.spoon;
		// 	grenade.spoon.unset();
		// }

		grenade_entity.get<components::sprite>().set(
			grenade.released_image_id
		);

		auto& rigid_body = grenade_entity.get<components::rigid_body>();
		
		//ensure(rigid_body.velocity().is_epsilon());

		rigid_body.set_velocity({ 0.f, 0.f });
		rigid_body.set_angular_velocity(0.f);
		rigid_body.apply_angular_impulse(1.5f * rigid_body.get_mass());
		rigid_body.apply_impulse(vec2().set_from_degrees(grenade_entity.get_logic_transform().rotation) * 5000 * rigid_body.get_mass());
		rigid_body.set_bullet_body(true);
		rigid_body.set_linear_damping(3.0f);

		auto& fixtures = grenade_entity.get<components::fixtures>();
		
		auto new_def = fixtures.get_raw_component();
		new_def.restitution = 0.6f;
		new_def.density = 10.f;
		new_def.material = assets::physical_material_id::GRENADE;
		
		fixtures = new_def;

		grenade_entity.get<components::shape_polygon>().set_activated(false);
		grenade_entity.get<components::shape_circle>().set_activated(true);
	}
}