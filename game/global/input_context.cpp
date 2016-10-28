#include "input_context.h"

void input_context::map_key_to_intent(augs::window::event::keys::key::key id, intent_type intent) {
	key_to_intent[id] = intent;
}

void input_context::map_event_to_intent(augs::window::event::message id, intent_type intent) {
	event_to_intent[id] = intent;
}
