#include "chase_system.h"
#include "entity_system/entity.h"
#include "../components/render_component.h"

void components::chase::set_target(augs::entity_id new_target) {
	target_newly_set = true;
	target = new_target;
}

void chase_system::process_entities(world&) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		auto& chase = it->get<components::chase>();

		if (chase.target.dead()) continue;
		
		if (chase.target_newly_set) {
			auto target_transform = chase.subscribe_to_previous ? chase.target->get<components::render>().previous_transform : chase.target->get<components::transform>();
			target_transform.rotation *= chase.rotation_multiplier;

			chase.previous = target_transform.pos;
			chase.rotation_previous = target_transform.rotation;
			chase.target_newly_set = false;
		}

		auto target_transform = chase.subscribe_to_previous ? chase.target->get<components::render>().previous_transform : chase.target->get<components::transform>();
		target_transform.rotation *= chase.rotation_multiplier;
		target_transform.pos = vec2i(target_transform.pos);

		if (chase.chase_type == components::chase::chase_type::OFFSET) {
			if (chase.relative) {
				chase.offset = transform.pos - chase.previous;
				chase.rotation_offset = transform.rotation - chase.rotation_previous;
			}

			transform.pos = target_transform.pos;
			transform.pos += chase.offset;

			if (chase.chase_rotation) {
				transform.rotation = target_transform.rotation;
				transform.rotation += chase.rotation_offset;
			}

			//transform.previous.pos = transform.pos;
			chase.previous = target_transform.pos;
			chase.rotation_previous = target_transform.rotation;
		}
		else if (chase.chase_type == components::chase::chase_type::ORBIT) {
			transform.pos = target_transform.pos + chase.rotation_orbit_offset;
			transform.pos.rotate(target_transform.rotation, target_transform.pos);
			
			if (chase.chase_rotation)
				transform.rotation = target_transform.rotation + chase.rotation_offset;
		}
		else if (chase.chase_type == components::chase::chase_type::PARALLAX) {
			transform.pos = chase.reference_position + (target_transform.pos - chase.target_reference_position) * chase.scrolling_speed;
		}
	}
}

