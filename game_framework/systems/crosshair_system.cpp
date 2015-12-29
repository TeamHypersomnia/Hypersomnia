#include "crosshair_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"

#include "../components/render_component.h"
#include "../resources/render_info.h"

void crosshair_system::consume_events() {
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::AIM) {
			auto transform = it.subject->find<components::transform>();
			auto crosshair = it.subject->find<components::crosshair>();

			if (!transform || !crosshair) continue;

			/* move crosshair according to its sensitivity and relative mouse movement (easier to support multiple resolutions) */
			transform->pos += vec2(it.state.mouse.rel * crosshair->sensitivity).rotate(crosshair->rotation_offset, vec2());

			//if (it.mouse_rel.non_zero()) {
			//	/* align the crosshair to bounds rect */
			//
			//	if(crosshair->bounds.good()) 
			//		crosshair->bounds.snap_point(transform->current.pos);
			//
			//	
			//}
		}
	}
}

void crosshair_system::process_entities() {
	for (auto it : targets) {
		auto render = it->find<components::render>();
		auto& crosshair = it->get<components::crosshair>();

		if (render && crosshair.should_blink) {
			float ratio = 0.f;
			crosshair.blink.animate(ratio);
			auto crosshair_sprite = static_cast < resources::sprite*>(render->model);
			if (crosshair_sprite)
				crosshair_sprite->size = vec2(crosshair_sprite->tex->get_size())*crosshair.size_multiplier*ratio;
		}
	}
}