#include "crosshair_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"
#include "../messages/crosshair_intent_message.h"

#include "../components/sprite_component.h"
#include "../components/camera_component.h"
#include "texture_baker/texture_baker.h"

void crosshair_system::generate_crosshair_intents() {
	parent_world.get_message_queue<messages::crosshair_intent_message>().clear();
	auto events = parent_world.get_message_queue<messages::intent_message>();

	for (auto& it : events) {
		if (it.intent == messages::intent_message::intent_type::MOVE_CROSSHAIR ||
			it.intent == messages::intent_message::intent_type::CROSSHAIR_PRIMARY_ACTION ||
			it.intent == messages::intent_message::intent_type::CROSSHAIR_SECONDARY_ACTION
			) {
			auto& subject = it.subject;
			auto crosshair = subject->find<components::crosshair>();

			if (!crosshair)
				continue;

			auto& transform = subject->get<components::transform>();

			vec2 delta = vec2(vec2(it.state.mouse.rel) * crosshair->sensitivity).rotate(crosshair->rotation_offset, vec2());

			vec2& base_offset = crosshair->base_offset;
			vec2 old_base_offset = base_offset;
			vec2 old_pos = transform.pos;

			base_offset += delta;

			auto constraints = crosshair->parent_camera->get<components::camera>()
				.get_constrained_crosshair_and_camera_offset(crosshair->parent_camera);

			base_offset = constraints.constrained_crosshair_base_offset;
			transform.pos = constraints.constrained_crosshair_pos;

			messages::crosshair_intent_message crosshair_intent;
			crosshair_intent.messages::intent_message::operator=(it);

			crosshair_intent.crosshair_world_pos = transform.pos;
			crosshair_intent.crosshair_world_rel = transform.pos - old_pos;
			crosshair_intent.crosshair_base_offset_rel = base_offset - old_base_offset;
			crosshair_intent.crosshair_base_offset = base_offset;

			/* we only care about crosshair movement resulting from the controller's movement,
			and not from camera's movement */
			if (it.intent != messages::intent_message::intent_type::MOVE_CROSSHAIR 
				|| crosshair_intent.crosshair_base_offset_rel.non_zero()) {
				parent_world.post_message(crosshair_intent);
			}
		}
	}
}

void crosshair_system::apply_crosshair_intents_to_crosshair_transforms() {
	auto& events = parent_world.get_message_queue<messages::crosshair_intent_message>();

	for (auto& it : events)
		it.subject->get<components::crosshair>().base_offset = it.crosshair_base_offset;


	for (auto it : targets) {
		auto& crosshair = it->get<components::crosshair>();
	
		auto player_id = crosshair.parent_camera->get<components::camera>().player;

		if (player_id.alive()) {
			it->get<components::transform>().pos = crosshair.base_offset + player_id->get<components::transform>().pos;
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