#include "crosshair_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"
#include "../messages/moved_message.h"

void crosshair_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.type == messages::intent_message::intent::AIM) {
			auto transform = it.subject->find<components::transform>();
			auto crosshair = it.subject->find<components::crosshair>();
			
			if (!transform || !crosshair) continue;

			/* move crosshair according to its sensitivity and relative mouse movement (easier to support multiple resolutions) */
			transform->current.pos += it.mouse_rel * crosshair->sensitivity;

			if (it.mouse_rel.non_zero()) {
				/* align the crosshair to bounds rect */

				crosshair->bounds.snap_point(transform->current.pos);
				owner.post_message(messages::moved_message(it.subject));
			}
		}
	}
}