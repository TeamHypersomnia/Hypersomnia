#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sprite_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/explosive_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/hand_fuse_component.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/shape_circle_component.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/assets/all_logical_assets.h"

void release_or_throw_fused_object(
	const logic_step step,
	const entity_handle fused_entity,
	const entity_id thrower_id,
	bool is_pressed_flag
) {
	auto& cosmos = step.get_cosmos();
#if TODO
	const auto& metas = step.get_logical_assets();
#endif
	const auto now = cosmos.get_timestamp();
	const auto thrower = cosmos[thrower_id];
	const auto thrower_transform = thrower.get_logic_transform();
	//const auto thrower_orientation = vec2::from_degrees(thrower_transform.rotation);
	
	// LOG("throrot: %x", thrower_transform.rotation);
	auto& fuse = fused_entity.get<components::hand_fuse>();
	auto& fuse_def = fused_entity.get<invariants::hand_fuse>();

	if (is_pressed_flag && !fuse.when_unpinned.was_set()) {
		fuse.when_unpinned = now;
		fuse_def.unpin_sound.start(
			step,
			sound_effect_start_input::fire_and_forget(thrower_transform).set_listener(thrower)
		);
	}
	else if (!is_pressed_flag && fuse.when_unpinned.was_set()) {
		const auto* const maybe_explosive = fused_entity.find<invariants::explosive>();

		if (maybe_explosive != nullptr) {
#if TODO
			const auto& explosive = *maybe_explosive;
#endif

			const auto total_impulse = [&]() {
				auto impulse = fuse_def.additional_release_impulse;

				if (const auto capability = thrower.find<invariants::item_slot_transfers>()) {
					impulse = impulse + capability->standard_throw_impulse;
				}

				return impulse;
			}();

			perform_transfer(
				item_slot_transfer_request{ fused_entity, inventory_slot_id(), -1, total_impulse, false }, 
				step
			);
			
			fuse_def.throw_sound.start(
				step,
				sound_effect_start_input::fire_and_forget(thrower_transform).set_listener(thrower)
			);

			/* const auto rigid_body = fused_entity.get<components::rigid_body>(); */

/* 			//ensure(rigid_body.get_velocity().is_epsilon()); */

/* 			rigid_body.set_velocity({ 0.f, 0.f }); */
/* 			rigid_body.set_angular_velocity(0.f); */
/* 			rigid_body.apply_angular_impulse(1.5f * rigid_body.get_mass()); */
/* 			rigid_body.apply_impulse(vec2::from_degrees(fused_entity.get_logic_transform().rotation) * fuse_def.standard_release_impulse * rigid_body.get_mass()); */

#if TODO
			rigid_body.set_linear_damping(3.0f);
			rigid_body.set_bullet_body(true);

			// TODO: this information will be specified by another type
			fused_entity.get<components::sprite>().set(
				explosive.released_image_id, metas
			);

			const auto fixtures = fused_entity.get<components::fixtures>();
			
			auto new_def = fixtures.get_raw_component();
			new_def.restitution = 0.6f;
			new_def.density = 10.f;

			const bool overwrite_physical_material = 
				explosive.released_physical_material.is_set()
			;

			if (overwrite_physical_material) {
				new_def.material = explosive.released_physical_material;
			}
			
			fixtures = new_def;

			fused_entity.get<components::shape_polygon>().set_activated(false);
			fused_entity.get<invariants::shape_circle>().set_activated(true);
#endif
		}
	}
}