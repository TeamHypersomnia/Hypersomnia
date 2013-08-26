#include "chase_system.h"
#include "entity_system/entity.h"

void chase_system::add(entity* e) {
	auto& chase = e->get<components::chase>();

	//if (chase.relative) {
		chase.previous = chase.target->get<components::transform>().current.pos;
		chase.rotation_previous = chase.target->get<components::transform>().current.rotation;
	//}

	processing_system::add(e);
}

void chase_system::process_entities(world&) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		auto& chase = it->get<components::chase>();

		if (chase.target == nullptr) continue;

		auto& target_transform = chase.target->get<components::transform>();

		if (chase.type == components::chase::chase_type::OFFSET) {
			if (chase.relative) {
				chase.offset = transform.current.pos - chase.previous;
				chase.rotation_offset = transform.current.rotation - chase.rotation_previous;
			}

			transform.current.pos = target_transform.current.pos;
			transform.current.pos += chase.offset;

			if (chase.chase_rotation) {
				transform.current.rotation = target_transform.current.rotation;
				transform.current.rotation += chase.rotation_offset;
			}

			//transform.previous.pos = transform.current.pos;
			chase.previous = target_transform.current.pos;
			chase.rotation_previous = target_transform.current.rotation;
		}
		else if (chase.type == components::chase::chase_type::ORBIT) {
			transform.current.pos = target_transform.current.pos + chase.rotation_orbit_offset;
			transform.current.pos.rotate(target_transform.current.rotation - chase.rotation_previous, target_transform.current.pos);
			transform.current.rotation = target_transform.current.rotation + chase.rotation_offset;
		}
	}
}

