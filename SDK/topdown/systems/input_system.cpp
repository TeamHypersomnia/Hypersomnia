#include "window_framework/window.h"
#include "entity_system/entity.h"

#include "input_system.h"

input_system::input_system(window::glwindow& input_window, bool& quit_flag) : input_window(input_window), quit_flag(quit_flag) {

}

void input_system::post(messages::intent_message incoming_event, world& owner) {
	auto& m = input_window.events.mouse;
	incoming_event.mouse_pos = m.pos;
	incoming_event.mouse_rel = m.rel;

	for (auto it = targets.begin(); it != targets.end(); ++it) {
		if ((*it)->get<components::input>().intents.find(incoming_event.type)) {
			incoming_event.subject = *it;
			owner.post_message(incoming_event);
		}
	}
}

bool input_system::post_intent_from_raw_id(world& owner, const context& active_context, unsigned id, bool state) {
	auto input_map = active_context.raw_id_to_intent;
	auto found_intent = input_map.find(id);

	if (found_intent != input_map.end()) {
		post(messages::intent_message((*found_intent).second, state), owner);
		return true;
	}
	
	return false;
}

void input_system::process_entities(world& owner) {
	using namespace messages;
	using namespace window::event;
	using namespace keys;

	window::event::message msg;
	unsigned incoming_event = 0;

	while (input_window.poll_events(msg)) {

		if (msg == window::event::close) {
			quit_flag = true;
		}

		for (auto it = active_contexts.begin(); it != active_contexts.end(); ++it) {
			bool succesfully_mapped = false;

			if (msg == key::down) {
				if (input_window.events.key == ESC)
					succesfully_mapped = quit_flag = true;
				else
					succesfully_mapped = post_intent_from_raw_id(owner, **it, input_window.events.key, true);
			}

			else if (msg == key::up) 
				succesfully_mapped = post_intent_from_raw_id(owner, **it, input_window.events.key, false);

			else if (msg == mouse::ldown)
				succesfully_mapped = post_intent_from_raw_id(owner, **it, mouse::ldown, true);
			else if (msg == mouse::lup)
				succesfully_mapped = post_intent_from_raw_id(owner, **it, mouse::ldown, false);
			else if (msg == mouse::rdown)
				succesfully_mapped = post_intent_from_raw_id(owner, **it, mouse::rdown, true);
			else if (msg == mouse::rup)
				succesfully_mapped = post_intent_from_raw_id(owner, **it, mouse::rdown, false);
			else
				succesfully_mapped = post_intent_from_raw_id(owner, **it, msg, true);

			if (succesfully_mapped) break;
		}

	}
}