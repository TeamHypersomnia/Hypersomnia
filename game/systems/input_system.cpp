#include <random>

#include "window_framework/window.h"
#include "game/entity_id.h"
#include "game/cosmos.h"

#include "input_system.h"

#include "augs/templates.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"
#include "game/components/input_receiver_component.h"
#include "game/messages/intent_message.h"

#include "game/messages/crosshair_intent_message.h"
#include "game/messages/gui_intents.h"

#include "window_framework/event.h"
#include "misc/step_player.h"
#include "game/step.h"
#include "game/entity_handle.h"

using namespace augs::window;


void input_system::post_unmapped_intents_from_raw_window_inputs(fixed_step& step, const machine_entropy& entropy) {
	step.messages.get_queue<messages::unmapped_intent_message>().clear();

	auto& context = cosmos.settings.input;
	auto& raw_inputs = entropy.local;

	for (auto& state : raw_inputs) {
		messages::unmapped_intent_message unmapped_intent;
		unmapped_intent.state = state;

		intent_type intent;

		bool found_context_entry = false;

		if (state.key_event == event::NONE) {
			unmapped_intent.pressed_flag = true;

			auto found_intent = context.event_to_intent.find(state.msg);
			if (found_intent != context.event_to_intent.end()) {
				intent = (*found_intent).second;
				found_context_entry = true;
			}
		}
		else if (state.key_event == event::PRESSED || state.key_event == event::RELEASED) {
			unmapped_intent.pressed_flag = state.key_event == event::PRESSED;

			auto found_intent = context.key_to_intent.find(state.key);
			if (found_intent != context.key_to_intent.end()) {
				intent = (*found_intent).second;
				found_context_entry = true;
			}
		}

		if (found_context_entry) {
			unmapped_intent.intent = intent;
			step.messages.post(unmapped_intent);

			break;
		}
	}
}

void input_system::map_unmapped_intents_to_entities(fixed_step& step) {
	for (auto& unmapped_intent : step.messages.get_queue<messages::unmapped_intent_message>()) {
		messages::intent_message entity_mapped_intent;
		entity_mapped_intent.unmapped_intent_message::operator=(unmapped_intent);

		for (auto it : cosmos.get(processing_subjects::WITH_INPUT_RECEIVER)) {
			entity_mapped_intent.subject = it;
			step.messages.post(entity_mapped_intent);
		}
	}
}

