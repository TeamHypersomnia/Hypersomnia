#include <random>

#include "augs/window_framework/window.h"
#include "game/entity_id.h"
#include "game/cosmos.h"

#include "input_system.h"

#include "augs/templates.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"
#include "game/components/input_receiver_component.h"
#include "game/messages/intent_message.h"

#include "game/messages/gui_intents.h"

#include "augs/window_framework/event.h"
#include "augs/misc/step_player.h"
#include "game/step.h"
#include "game/entity_handle.h"

using namespace augs::window;

void input_system::make_intents_from_raw_entropy(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& context = cosmos.settings.input;

	for (auto& per_entity : step.entropy.entropy_per_entity) {
		for (auto& raw : per_entity.second) {
			messages::intent_message mapped_intent;
			mapped_intent.subject = per_entity.first;
			mapped_intent.state = raw;

			intent_type intent;

			bool found_context_entry = false;

			if (raw.key_event == event::NO_CHANGE) {
				mapped_intent.pressed_flag = true;

				auto found_intent = context.event_to_intent.find(raw.msg);
				if (found_intent != context.event_to_intent.end()) {
					intent = (*found_intent).second;
					found_context_entry = true;
				}
			}
			else if (raw.key_event == event::PRESSED || raw.key_event == event::RELEASED) {
				mapped_intent.pressed_flag = raw.key_event == event::PRESSED;

				auto found_intent = context.key_to_intent.find(raw.key);
				if (found_intent != context.key_to_intent.end()) {
					intent = (*found_intent).second;
					found_context_entry = true;
				}
			}

			if (found_context_entry) {
				mapped_intent.intent = intent;
				step.messages.post(mapped_intent);
			}
		}
	}
}