#include "position_copying_system.h"
#include "game/entity_id.h"
#include "game/components/render_component.h"

void components::position_copying::set_target(entity_id new_target) {
	target_newly_set = true;
	target = new_target;
}

void position_copying_system::update_transforms() {
	for (auto it : targets) {
		auto& transform = it.get<components::transform>();
		auto& position_copying = it.get<components::position_copying>();

		if (position_copying.target.dead()) continue;
		
		if (position_copying.target_newly_set) {
			auto target_transform = position_copying.subscribe_to_previous ? position_copying.target.get<components::render>().previous_transform : position_copying.target.get<components::transform>();
			target_transform.rotation *= position_copying.rotation_multiplier;

			position_copying.previous = target_transform.pos;
			position_copying.rotation_previous = target_transform.rotation;
			position_copying.target_newly_set = false;
		}

		auto target_transform = position_copying.subscribe_to_previous ? position_copying.target.get<components::render>().previous_transform : position_copying.target.get<components::transform>();
		target_transform.rotation *= position_copying.rotation_multiplier;
		target_transform.pos = vec2i(target_transform.pos);

		if (position_copying.position_copying_type == components::position_copying::position_copying_type::OFFSET) {
			if (position_copying.relative) {
				position_copying.offset = transform.pos - position_copying.previous;
				position_copying.rotation_offset = transform.rotation - position_copying.rotation_previous;
			}

			transform.pos = target_transform.pos;
			transform.pos += position_copying.offset;

			if (position_copying.position_copying_rotation) {
				transform.rotation = target_transform.rotation;
				transform.rotation += position_copying.rotation_offset;
			}

			//transform.previous.pos = transform.pos;
			position_copying.previous = target_transform.pos;
			position_copying.rotation_previous = target_transform.rotation;
		}
		else if (position_copying.position_copying_type == components::position_copying::position_copying_type::ORBIT) {
			transform.pos = target_transform.pos + position_copying.rotation_orbit_offset;
			transform.pos.rotate(target_transform.rotation, target_transform.pos);
			
			if (position_copying.position_copying_rotation)
				transform.rotation = target_transform.rotation + position_copying.rotation_offset;
		}
		else if (position_copying.position_copying_type == components::position_copying::position_copying_type::PARALLAX) {
			transform.pos = position_copying.reference_position + (target_transform.pos - position_copying.target_reference_position) * position_copying.scrolling_speed;
		}
	}
}

