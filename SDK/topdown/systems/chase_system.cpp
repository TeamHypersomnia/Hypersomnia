#include "chase_system.h"

void chase_system::process_entities(world&) {
	for (auto it : targets) {
		auto& transform = it->get<components::transform>();
		
		transform.current.pos = it->get<components::chase>().target->get<components::transform>().current.pos;
	}
}
