#include "window_framework/window.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/raw_window_input_message.h"

#include "input_system.h"
#include <iostream>

#include "augs/misc/stream.h"

using namespace augs::window;

input_system::input_system(world& parent_world) : processing_system_templated(parent_world),
	raw_window_input_player(parent_world),
	crosshair_intent_player(parent_world)
{
}

input_system::context::context() : enabled(true) {
}

void input_system::context::map_key_to_intent(window::event::keys::key id, messages::intent_message::intent_type intent) {
	key_to_intent[id].push_back(intent);
}

void input_system::context::map_event_to_intent(window::event::message id, messages::intent_message::intent_type intent) {
	event_to_intent[id].push_back(intent);
}

void input_system::add_context(context c) {
	active_contexts.push_back(c);
}

void input_system::clear_contexts() {
	active_contexts.clear();
}

//void input_system::inputs_per_step::serialize(std::ofstream& f) {
//	augs::serialize_vector(f, events);
//}
//
//bool input_system::inputs_per_step::should_serialize() {
//	return !events.empty();
//}
//
//void input_system::inputs_per_step::deserialize(std::ifstream& f) {
//	augs::deserialize_vector(f, events);
//}

void input_system::post_intents_from_inputs(const std::vector<messages::raw_window_input_message>& inputs_for_this_step) {
	if (!active_contexts.empty()) {
		for (auto& it : inputs_for_this_step) {
			auto& state = it.raw_window_input;

			for (auto& context : active_contexts) {
				if (!context.enabled) continue;

				messages::unmapped_intent_message unmapped_intent;
				unmapped_intent.state = state;

				std::vector<messages::intent_message::intent_type> intents;

				bool found_context_entry = false;

				if (state.key_event == event::NONE) {
					unmapped_intent.pressed_flag = true;

					auto found_intent = context.event_to_intent.find(state.msg);
					if (found_intent != context.event_to_intent.end()) {
						intents = (*found_intent).second;
						found_context_entry = true;
					}
				}
				else if (state.key_event == event::PRESSED || state.key_event == event::RELEASED) {
					unmapped_intent.pressed_flag = state.key_event == event::PRESSED;

					auto found_intent = context.key_to_intent.find(state.key);
					if (found_intent != context.key_to_intent.end()) {
						intents = (*found_intent).second;
						found_context_entry = true;
					}
				}

				if (found_context_entry) {
					unmapped_intent.intent.intents = intents;
					parent_world.post_message(unmapped_intent);

					messages::intent_message entity_mapped_intent;
					entity_mapped_intent.unmapped_intent_message::operator=(unmapped_intent);

					for (auto it = targets.begin(); it != targets.end(); ++it) {
						//if ((*it)->get<components::input>().intents.find(unmapped_intent.intent)) {
							entity_mapped_intent.subject = *it;
							parent_world.post_message(entity_mapped_intent);
						//}
					}

					break;
				}
			}
		}
	}
}

void input_system::acquire_raw_window_inputs() {
	raw_window_input_player.acquire_events_from_rendering_time();
}

void input_system::acquire_events_from_rendering_time() {
	crosshair_intent_player.acquire_events_from_rendering_time();
}

void input_system::replay_rendering_time_events_passed_to_last_logic_step() {
	crosshair_intent_player.pass_last_unpacked_logic_events_for_rendering_time_approximation();
}

void input_system::post_input_intents_for_rendering_time() {
	///* if we are replaying, let's pass some mouse strokes registered during logic step time
	//	just that some of the original mouse movements appear in the recordings
	//*/
	//if (!raw_window_input_player.player.is_replaying()) {
	//}

	parent_world.get_message_queue<messages::unmapped_intent_message>().clear();
	parent_world.get_message_queue<messages::intent_message>().clear();

	post_intents_from_inputs(raw_window_input_player.inputs_from_last_rendering_time.events);
}
	
void input_system::post_input_intents_for_logic_step() {
	parent_world.get_message_queue<messages::unmapped_intent_message>().clear();
	parent_world.get_message_queue<messages::intent_message>().clear();

	// record/replay total entropia
	raw_window_input_player.biserialize();

	post_intents_from_inputs(raw_window_input_player.buffered_inputs_for_next_step.events);

	raw_window_input_player.clear_step();
}

void input_system::post_rendering_time_events_for_logic_step() {
	crosshair_intent_player.generate_events_for_logic_step();
}
