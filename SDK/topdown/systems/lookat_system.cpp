#include "lookat_system.h"
#include "../messages/moved_message.h"

void lookat_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& lookat = it->get<components::lookat>();
		
		auto target_transform = lookat.target->find<components::transform>();
		
		if (target_transform) {
			auto& transform = it->get<components::transform>();

			transform.current.rotation = (target_transform->current.pos - transform.current.pos).get_radians();
			//owner.post_message(messages::moved_message(it));
		}
	}
}
