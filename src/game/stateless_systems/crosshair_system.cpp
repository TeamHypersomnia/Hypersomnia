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
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();

	{
		const auto& events = step.transient.messages.get_queue<messages::motion_message>();
		
		for (const auto& it : events) {
			const auto subject = cosmos[it.subject];

			if (!subject.has<components::crosshair>()) {
				continue;
			}

			auto& crosshair = subject.get<components::crosshair>();

			const vec2 delta = vec2(vec2(it.offset) * crosshair.sensitivity).rotate(crosshair.rotation_offset, vec2());

			vec2& base_offset = crosshair.base_offset;
			const vec2 old_base_offset = base_offset;
			const vec2 old_pos = subject.get_logic_transform().pos;

			base_offset += delta;
			base_offset.clamp_rotated(crosshair.get_bounds_in_this_look(), crosshair.rotation_offset);

			messages::crosshair_motion_message crosshair_motion;

			crosshair_motion.subject = it.subject;
			crosshair_motion.crosshair_base_offset_rel = base_offset - old_base_offset;
			crosshair_motion.crosshair_base_offset = base_offset;
			crosshair_motion.crosshair_world_pos = base_offset + cosmos[crosshair.character_entity_to_chase].get_logic_transform().pos;

			step.transient.messages.post(crosshair_motion);
		}
	}

	const auto& events = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		const auto subject = cosmos[it.subject];
		
		if (!subject.has<components::crosshair>()) {
			continue;
		}

		auto& crosshair = subject.get<components::crosshair>();

		if (it.intent == game_intent_type::SWITCH_LOOK && it.was_pressed()) {
			auto& mode = crosshair.orbit_mode;

			if (mode == components::crosshair::LOOK) {
				mode = components::crosshair::ANGLED;
			}
			else {
				mode = components::crosshair::LOOK;
			}
		}
	}
}
void crosshair_system::apply_crosshair_intents_to_base_offsets(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<messages::crosshair_motion_message>();

	for (const auto& it : events) {
		cosmos[it.subject].get<components::crosshair>().base_offset = it.crosshair_base_offset;
	}
}

void crosshair_system::apply_base_offsets_to_crosshair_transforms(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	
	cosmos.for_each(
		processing_subjects::WITH_CROSSHAIR,
		[&](const auto it) {
			const auto player = cosmos[it.get<components::crosshair>().character_entity_to_chase];

			if (player.alive()) {
				const vec2 aiming_displacement = components::crosshair::calculate_aiming_displacement(it, true);
				const vec2 player_center = player.get_logic_transform().pos;

				it.get<components::transform>().pos = aiming_displacement + player_center;
			}
		}
	);
}