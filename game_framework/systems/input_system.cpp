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

void input_system::context::set_intent(unsigned raw_id, messages::intent_message::intent_type intent) {
	raw_id_to_intent[raw_id] = intent;
}

void input_system::add_context(context* c) {
	active_contexts.push_back(c);
}

void input_system::clear_contexts() {
	active_contexts.clear();
}

void input_system::inputs_per_step::serialize(std::ofstream& f) {
	augs::serialize_vector(f, events);
}

void input_system::inputs_per_step::deserialize(std::ifstream& f) {
	augs::deserialize_vector(f, events);
}

void input_system::generate_input_intents_for_next_step() {
	inputs_per_step inputs_for_this_step;

	for (auto& m : parent_world.get_message_queue<messages::raw_window_input_message>())
		inputs_for_this_step.events.push_back(m.raw_window_input);
	
	// record/replay total entropia
	player.biserialize(inputs_for_this_step);

	if (!active_contexts.empty()) {
		for(auto& state : inputs_for_this_step.events) {
			for (auto it : active_contexts) {
				if (!it->enabled) continue;

				unsigned intent_searched;
				bool pressed_flag;

				if (state.key_event == event::PRESSED) {
					intent_searched = state.key;
					pressed_flag = true;
				}
				else if (state.key_event == event::RELEASED) {
					intent_searched = state.key;
					pressed_flag = false;
				}
				else {
					intent_searched = state.msg;
					pressed_flag = true;
				}

				auto input_map = it->raw_id_to_intent;
				auto found_intent = input_map.find(intent_searched);

				if (found_intent != input_map.end()) {
					messages::intent_message intent;

					intent.intent = (*found_intent).second;
					intent.pressed_flag = pressed_flag;
					intent.state = state;

					for (auto it = targets.begin(); it != targets.end(); ++it) {
						if ((*it)->get<components::input>().intents.find(intent.intent)) {
							intent.subject = *it;
							parent_world.post_message(intent);
						}
					}

					break;
				}
			}
		}
	}
}