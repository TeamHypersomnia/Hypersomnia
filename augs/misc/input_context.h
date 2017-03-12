#pragma once
#include <unordered_map>
#include "game/enums/intent_type.h"
#include "augs/window_framework/event.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/key_and_mouse_intent.h"

#include "augs/misc/machine_entropy.h"

template <class intent_enum_type>
struct basic_input_context {
	augs::enum_associative_array<augs::window::event::keys::key, intent_enum_type> key_to_intent;
	augs::enum_associative_array<augs::window::event::message, intent_enum_type> event_to_intent;

	void map_key_to_intent(const augs::window::event::keys::key, const intent_enum_type);
	void map_event_to_intent(const augs::window::event::message, const intent_enum_type);

	augs::window::event::keys::key get_bound_key_if_any(const intent_enum_type) const;

	key_and_mouse_intent_vector to_key_and_mouse_intents(const augs::machine_entropy::local_type&) const;
};

typedef basic_input_context<intent_type> input_context;