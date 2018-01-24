#include "position_copying_system.h"
#include "game/transcendental/entity_id.h"
#include "game/components/render_component.h"

#include "game/components/position_copying_component.h"
#include "game/components/transform_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/transcendental/cosmos.h"

using namespace augs;

void components::position_copying::set_target(const const_entity_handle new_target) {
	target_newly_set = true;
	target = new_target;
}

components::position_copying components::position_copying::configure_chasing(
	const const_entity_handle target, 
	const components::transform chaser_place_of_birth, 
	const chasing_configuration cfg
) {
	components::position_copying copying;
	
	copying.previous = chaser_place_of_birth;
	copying.set_target(target);

	const auto target_transform = target.get_logic_transform();
	copying.position_copying_mode = components::position_copying::position_copying_type::ORBIT;

	copying.position_copying_rotation = true;
	copying.rotation_offset = chaser_place_of_birth.rotation - target_transform.rotation;
	copying.orbit_offset = (chaser_place_of_birth.pos - target_transform.pos).rotate(-target_transform.rotation, vec2(0.f, 0.f));

	return copying;
}

components::transform components::position_copying::get_previous_transform() const {
	return previous;
}

void position_copying_system::update_transforms(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();

	cosmos.for_each(
		processing_subjects::WITH_POSITION_COPYING,
		[&](const entity_handle it) {
			components::transform transform = it.get_logic_transform();
			auto& position_copying = it.get<components::position_copying>();

			position_copying.previous = transform;

			if (cosmos[position_copying.target].dead()) {
				return;
			}

			auto target_transform = cosmos[position_copying.target].get_logic_transform();
			target_transform.rotation *= position_copying.rotation_multiplier;
			target_transform.pos.discard_fract();

			else if (position_copying.position_copying_mode == components::position_copying::position_copying_type::ORBIT) {
				transform.pos = target_transform.pos + position_copying.orbit_offset;
				transform.pos.rotate(target_transform.rotation, target_transform.pos);
				
				if (position_copying.position_copying_rotation) {
					transform.rotation = target_transform.rotation + position_copying.rotation_offset;
				}
			}

			if (position_copying.target_newly_set) {
				position_copying.previous = transform;
				position_copying.target_newly_set = false;
			}

			it.set_logic_transform(step, transform);
		}
	);
}

