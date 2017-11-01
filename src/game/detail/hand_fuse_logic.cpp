#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sprite_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/explosive_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/all_inferred_state_component.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/assets/all_logical_assets.h"

void release_or_throw_fused_object(
	const logic_step step,
	const entity_handle fused_entity,
	const entity_id thrower_id,
	bool is_pressed_flag
) {
	auto& cosmos = step.cosm;
	const auto& metas = step.input.logical_assets;
	const auto now = cosmos.get_timestamp();
	const auto delta = step.get_delta();
	const auto thrower = cosmos[thrower_id];
	const auto thrower_transform = thrower.get_logic_transform();
	//const auto thrower_orientation = vec2::from_degrees(thrower_transform.rotation);
	
	// LOG("throrot: %x", thrower_transform.rotation);
	auto& fuse = fused_entity.get<components::hand_fuse>();

	if (is_pressed_flag && !fuse.when_released.was_set()) {
		fused_entity.get<components::processing>().enable_in(processing_subjects::WITH_HAND_FUSE);

		fuse.when_released = now;
		fuse.when_detonates.step = static_cast<unsigned>(now.step + (1 / delta.in_seconds() * 1.0));

		sound_existence_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = thrower;
		in.effect = fuse.unpin_sound;

		in.create_sound_effect_entity(step, thrower_transform, thrower).add_standard_components(step);
	}
	else if (!is_pressed_flag && fuse.when_released.was_set()) {
		auto* const maybe_explosive = fused_entity.find<components::explosive>();

		if (maybe_explosive != nullptr) {
			auto& explosive = *maybe_explosive;

			perform_transfer(
				item_slot_transfer_request{ fused_entity, inventory_slot_id(), -1, false, 0.f }, 
				step
			);
			
			sound_existence_input in;
			in.delete_entity_after_effect_lifetime = true;
			in.direct_listener = thrower;
			in.effect = fuse.throw_sound;
			
			in.create_sound_effect_entity(step, thrower_transform, thrower).add_standard_components(step);

			fused_entity.get<components::sprite>().set(
				explosive.released_image_id, metas
			);

			const auto rigid_body = fused_entity.get<components::rigid_body>();
			
			//ensure(rigid_body.velocity().is_epsilon());

			rigid_body.set_velocity({ 0.f, 0.f });
			rigid_body.set_angular_velocity(0.f);
			rigid_body.apply_angular_impulse(1.5f * rigid_body.get_mass());
			rigid_body.apply_impulse(vec2::from_degrees(fused_entity.get_logic_transform().rotation) * 5000 * rigid_body.get_mass());
			rigid_body.set_bullet_body(true);
			rigid_body.set_linear_damping(3.0f);

			const auto fixtures = fused_entity.get<components::fixtures>();
			
			auto new_def = fixtures.get_raw_component();
			new_def.restitution = 0.6f;
			new_def.density = 10.f;

			const bool overwrite_physical_material = 
				explosive.released_physical_material != assets::physical_material_id::INVALID
			;

			if (overwrite_physical_material) {
				new_def.material = explosive.released_physical_material;
			}
			
			fixtures = new_def;

			fused_entity.get<components::shape_polygon>().set_activated(false);
			fused_entity.get<components::shape_circle>().set_activated(true);
		}
	}
}