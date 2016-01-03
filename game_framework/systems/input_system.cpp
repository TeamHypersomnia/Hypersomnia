#include "window_framework/window.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/raw_window_input_message.h"

#include "input_system.h"
#include <iostream>

#include "utilities/misc/stream.h"

using namespace augs::window;

input_system::context::context() : enabled(true) {
}

void input_system::clear() {
	active_contexts.clear();
	processing_system::clear();
}


void input_system::context::map_key_to_intent(window::event::keys::key id, messages::intent_message::intent_type intent) {
	key_to_intent[id] = intent;
}

void input_system::context::map_event_to_intent(window::event::message id, messages::intent_message::intent_type intent) {
	event_to_intent[id] = intent;
}

void input_system::add_context(context c) {
	active_contexts.push_back(c);
}

void input_system::clear_contexts() {
	active_contexts.clear();
}

void input_system::inputs_per_step::serialize(std::ofstream& f) {
	augs::serialize_vector(f, events);
}

bool input_system::inputs_per_step::should_serialize() {
	return !events.empty();
}

void input_system::inputs_per_step::deserialize(std::ifstream& f) {
	augs::deserialize_vector(f, events);
}

void input_system::generate_input_intents_for_next_step() {
	parent_world.get_message_queue<messages::unmapped_intent_message>().clear();
	parent_world.get_message_queue<messages::intent_message>().clear();

	inputs_per_step inputs_for_this_step;

	for (auto& m : parent_world.get_message_queue<messages::raw_window_input_message>())
		inputs_for_this_step.events.push_back(m.raw_window_input);
	
	// record/replay total entropia
	player.biserialize(inputs_for_this_step);

	if (!active_contexts.empty()) {
		for(auto& state : inputs_for_this_step.events) {
			for (auto it : active_contexts) {
				if (!it.enabled) continue;

				messages::unmapped_intent_message intent;
				intent.state = state;

				bool found_context_entry = false;

				if (state.key_event == event::NONE) {
					intent.pressed_flag = true;

					auto found_intent = it.event_to_intent.find(state.msg);
					if (found_intent != it.event_to_intent.end()) {
						intent.intent = (*found_intent).second;
						found_context_entry = true;
					}
				}
				else if (state.key_event == event::PRESSED || state.key_event == event::RELEASED) {
					intent.pressed_flag = state.key_event == event::PRESSED;

					auto found_intent = it.key_to_intent.find(state.key);
					if (found_intent != it.key_to_intent.end()) {
						intent.intent = (*found_intent).second;
						found_context_entry = true;
					}
				}

				if (found_context_entry) {
					parent_world.post_message(intent);

					messages::intent_message mapped_intent;
					mapped_intent.unmapped_intent_message::operator=(intent);

					for (auto it = targets.begin(); it != targets.end(); ++it) {
						if ((*it)->get<components::input>().intents.find(intent.intent)) {
							mapped_intent.subject = *it;
							parent_world.post_message(mapped_intent);
						}
					}

					break;
				}
			}
		}
	}

	parent_world.get_message_queue<messages::raw_window_input_message>().clear();
}