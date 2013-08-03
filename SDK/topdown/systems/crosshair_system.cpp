#include "crosshair_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"

void crosshair_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it = events.begin(); it != events.end(); ++it) {
		if ((*it).type == messages::intent_message::intent::AIM) {
			auto& transform = (*it).subject->get<components::transform>();
			auto& crosshair = (*it).subject->get<components::crosshair>();

			/* move crosshair according to its sensitivity and relative mouse movement (easier to support multiple resolutions) */
			transform.current.pos += (*it).mouse_rel * crosshair.sensitivity;

			/* align the crosshair to bounds rect */
			crosshair.bounds.snap_point(transform.current.pos);
		}
	}
}