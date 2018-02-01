#include "crosshair_system.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"
#include "game/messages/crosshair_motion_message.h"

#include "game/components/sprite_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/transform_component.h"
#include "game/messages/intent_message.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

vec2 components::crosshair::get_bounds_in_this_look() const {
	if (orbit_mode == ANGLED) {
		return base_offset_bound / 2.f;
	}
	else if (orbit_mode == LOOK) {
		return base_offset_bound;
	}

	return {};
}

void crosshair_system::generate_crosshair_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();

	{
		const auto& events = step.get_queue<messages::motion_message>();
		
		for (const auto& it : events) {
			const auto subject = cosmos[it.subject];

			if (const auto crosshair = subject.find_crosshair()) {
				const vec2 delta = vec2(vec2(it.offset) * crosshair->sensitivity).rotate(crosshair->rotation_offset, vec2());

				vec2& base_offset = crosshair->base_offset;
				const vec2 old_base_offset = base_offset;

				base_offset += delta;
				base_offset.clamp_rotated(crosshair->get_bounds_in_this_look(), crosshair->rotation_offset);

				messages::crosshair_motion_message crosshair_motion;

				crosshair_motion.subject = it.subject;
				crosshair_motion.crosshair_base_offset_rel = base_offset - old_base_offset;
				crosshair_motion.crosshair_base_offset = base_offset;

				step.post_message(crosshair_motion);
			}
		}
	}

	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		const auto subject = cosmos[it.subject];
		
		if (const auto crosshair = subject.find_crosshair()) {
			if (it.intent == game_intent_type::SWITCH_LOOK && it.was_pressed()) {
				auto& mode = crosshair->orbit_mode;

				if (mode == components::crosshair::LOOK) {
					mode = components::crosshair::ANGLED;
				}
				else {
					mode = components::crosshair::LOOK;
				}
			}
		}
	}
}

void crosshair_system::apply_crosshair_intents_to_base_offsets(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& events = step.get_queue<messages::crosshair_motion_message>();

	for (const auto& it : events) {
		cosmos[it.subject].find_crosshair()->base_offset = it.crosshair_base_offset;
	}
}