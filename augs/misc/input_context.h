#pragma once
#include <unordered_map>
#include "game/enums/intent_type.h"
#include "augs/window_framework/event.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/key_and_mouse_intent.h"

#include "augs/misc/machine_entropy.h"

struct input_context {
	augs::enum_associative_array<augs::window::event::keys::key, intent_type> key_to_intent;
	augs::enum_associative_array<augs::window::event::message, intent_type> event_to_intent;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(key_to_intent),
			CEREAL_NVP(event_to_intent)
		);
	}

	void map_key_to_intent(const augs::window::event::keys::key, const intent_type);
	void map_event_to_intent(const augs::window::event::message, const intent_type);

	augs::window::event::keys::key get_bound_key_if_any(const intent_type) const;

	std::vector<key_and_mouse_intent> to_key_and_mouse_intents(const augs::machine_entropy::local_type&) const;
};