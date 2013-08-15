#include "chase_system.h"

void chase_system::add(entity* e) {
	auto& chase = e->get<components::chase>();
	
	if (chase.relative) {
		chase.previous = chase.target->get<components::transform>().current.pos;
	}

	processing_system::add(e);
}

void chase_system::process_entities(world&) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		auto& chase = it->get<components::chase>();
		auto& target_transform = chase.target->get<components::transform>();

		if (chase.relative) 
			chase.offset = transform.current.pos - chase.previous;

		transform.current.pos = target_transform.current.pos;
		transform.current.pos += chase.offset;

		//transform.previous.pos = transform.current.pos;
		chase.previous = target_transform.current.pos;
	}
}

