#include "crosshair_system.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/intent_message.h"
#include "game/messages/crosshair_intent_message.h"

#include "game/components/sprite_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/transform_component.h"
#include "game/messages/intent_message.h"

#include "augs/texture_baker/texture_baker.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

void components::crosshair::update_bounds() {
	max_look_expand = visible_world_area / 2;

	if (orbit_mode == ANGLED)
		bounds_for_base_offset = visible_world_area / 2.f;
	if (orbit_mode == LOOK)
		bounds_for_base_offset = max_look_expand + visible_world_area / 2;
}

void crosshair_system::generate_crosshair_intents(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	step.messages.get_queue<messages::crosshair_intent_message>().clear();
	auto events = step.messages.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		auto subject = cosmos[it.subject];
		
		if (!subject.has<components::crosshair>())
			continue;

		auto& crosshair = subject.get<components::crosshair>();

		messages::crosshair_intent_message crosshair_intent;
		crosshair_intent.messages::intent_message::operator=(it);

		if (it.uses_mouse_motion()) {
			vec2 delta = vec2(vec2(it.mouse_rel) * crosshair.sensitivity).rotate(crosshair.rotation_offset, vec2());

			vec2& base_offset = crosshair.base_offset;
			vec2 old_base_offset = base_offset;
			vec2 old_pos = position(subject);

			base_offset += delta;
			base_offset.clamp_rotated(crosshair.bounds_for_base_offset, crosshair.rotation_offset);

			crosshair_intent.crosshair_base_offset_rel = base_offset - old_base_offset;
			crosshair_intent.crosshair_base_offset = base_offset;
			crosshair_intent.crosshair_world_pos = base_offset + cosmos[crosshair.character_entity_to_chase].logic_transform().pos;

			step.messages.post(crosshair_intent);
		}
		else if (it.intent == intent_type::SWITCH_LOOK && it.pressed_flag) {
			auto& mode = crosshair.orbit_mode;

			if (mode == components::crosshair::LOOK)
				mode = components::crosshair::ANGLED;
			else mode = components::crosshair::LOOK;

			crosshair.update_bounds();
		}
	}
}
void crosshair_system::apply_crosshair_intents_to_base_offsets(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& events = step.messages.get_queue<messages::crosshair_intent_message>();

	for (const auto& it : events)
		cosmos[it.subject].get<components::crosshair>().base_offset = it.crosshair_base_offset;
}

void crosshair_system::apply_base_offsets_to_crosshair_transforms(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	for (auto& it : cosmos.get(processing_subjects::WITH_CROSSHAIR)) {
		auto player_id = cosmos[it.get<components::crosshair>().character_entity_to_chase];

		if (player_id.alive()) {
			vec2 aiming_displacement = components::crosshair::calculate_aiming_displacement(it, true);
			vec2 player_center = position(player_id);

			it.get<components::transform>().pos = aiming_displacement + player_center;
		}
	}
}