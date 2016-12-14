#include "position_copying_system.h"
#include "game/transcendental/entity_id.h"
#include "game/components/render_component.h"

#include "game/components/position_copying_component.h"
#include "game/components/transform_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "game/transcendental/cosmos.h"

using namespace augs;

void components::position_copying::set_target(const entity_id new_target) {
	target_newly_set = true;
	target = new_target;
}

void components::position_copying::configure_chasing(const const_entity_handle subject, const components::transform chaser_place_of_birth, const chasing_configuration cfg) {
	set_target(subject.get_id());

	if (cfg == chasing_configuration::RELATIVE_ORBIT) {
		const auto subject_transform = subject.logic_transform();
		position_copying_mode = components::position_copying::position_copying_type::ORBIT;

		position_copying_rotation = true;
		rotation_offset = chaser_place_of_birth.rotation - subject_transform.rotation;
		rotation_orbit_offset = (chaser_place_of_birth.pos - subject_transform.pos).rotate(-subject_transform.rotation, vec2(0.f, 0.f));
	}
}

components::transform components::position_copying::get_previous_transform() const {
	return previous;
}

void position_copying_system::update_transforms(logic_step& step) {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	
	for (const auto& it : cosmos.get(processing_subjects::WITH_POSITION_COPYING)) {
		components::transform transform = it.logic_transform();
		auto& position_copying = it.get<components::position_copying>();

		position_copying.previous = transform;

		if (cosmos[position_copying.target].dead()) continue;
		
		if (position_copying.target_newly_set) {
			auto target_transform = cosmos[position_copying.target].logic_transform();
			target_transform.rotation *= position_copying.rotation_multiplier;
			
			position_copying.target_newly_set = false;
		}

		auto target_transform = cosmos[position_copying.target].logic_transform();
		target_transform.rotation *= position_copying.rotation_multiplier;
		target_transform.pos = vec2i(target_transform.pos);

		if (position_copying.position_copying_mode == components::position_copying::position_copying_type::OFFSET) {
			transform.pos = target_transform.pos;
			transform.pos += position_copying.offset;

			if (position_copying.position_copying_rotation) {
				transform.rotation = target_transform.rotation;
				transform.rotation += position_copying.rotation_offset;
			}
		}
		else if (position_copying.position_copying_mode == components::position_copying::position_copying_type::ORBIT) {
			transform.pos = target_transform.pos + position_copying.rotation_orbit_offset;
			transform.pos.rotate(target_transform.rotation, target_transform.pos);
			
			if (position_copying.position_copying_rotation) {
				transform.rotation = target_transform.rotation + position_copying.rotation_offset;
			}
		}
		else if (position_copying.position_copying_mode == components::position_copying::position_copying_type::PARALLAX) {
			transform.pos = position_copying.reference_position + (target_transform.pos - position_copying.target_reference_position) * position_copying.scrolling_speed;
		}

		it.set_logic_transform(transform);
	}
}

