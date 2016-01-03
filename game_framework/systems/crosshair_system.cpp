#include "crosshair_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"

#include "../components/sprite_component.h"
#include "texture_baker/texture_baker.h"

void crosshair_system::react_to_aiming_intents() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::AIM) {
			auto transform = it.subject->find<components::transform>();
			auto crosshair = it.subject->find<components::crosshair>();

			if (!transform || !crosshair) continue;

			/* move crosshair according to its sensitivity and relative mouse movement (easier to support multiple resolutions) */
			transform->pos += vec2(it.state.mouse.rel * crosshair->sensitivity).rotate(crosshair->rotation_offset, vec2());
		}
	}
}

void crosshair_system::animate_crosshair_sizes() {
	for (auto it : targets) {
		auto& crosshair = it->get<components::crosshair>();

		if (crosshair.should_blink) {
			float ratio = 0.f;
			crosshair.blink.animate(ratio);

			auto* crosshair_sprite = it->find<components::sprite>();
			
			if (crosshair_sprite) {
				crosshair_sprite->update_size();
				crosshair_sprite->size *= crosshair.size_multiplier*ratio;
			}
		}
	}
}