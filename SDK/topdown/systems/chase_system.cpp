#include "stdafx.h"
#include "chase_system.h"
#include "entity_system/entity.h"

void components::chase::set_target(augmentations::entity_system::entity* new_target) {
	target_newly_set = true;
	target.set(new_target);
}

void chase_system::process_entities(world&) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>().current;
		auto& chase = it->get<components::chase>();

		if (chase.target == nullptr) continue;
		
		if (chase.target_newly_set) {
			auto& target_chase = chase.target->get<components::transform>().current;
			chase.previous = target_chase.pos;
			chase.rotation_previous = target_chase.rotation;
			chase.target_newly_set = false;
		}

		auto& target_transform = chase.target->get<components::transform>().current;

		if (chase.type == components::chase::chase_type::OFFSET) {
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
		else if (chase.type == components::chase::chase_type::ORBIT) {
			transform.pos = target_transform.pos + chase.rotation_orbit_offset;
			transform.pos.rotate(target_transform.rotation, target_transform.pos);
			transform.rotation = target_transform.rotation + chase.rotation_offset;
		}
	}
}

