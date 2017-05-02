#pragma once
#include "augs/window_framework/event.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/basic_game_intent.h"

#include "augs/misc/machine_entropy.h"

template <class intent_enum_type>
struct basic_input_context {
	typedef augs::window::event::keys::key key_type;
	typedef augs::window::event::message message_type;

	augs::enum_associative_array<key_type, intent_enum_type> key_to_intent;
	augs::enum_associative_array<message_type, intent_enum_type> event_to_intent;

	key_type get_bound_key_if_any(const intent_enum_type intent) const {
		for (const auto& k : key_to_intent) {
			if (k.second == intent) {
				return k.first;
			}
		}

		return key_type::INVALID;
	}

	void map_key_to_intent(const key_type id, const intent_enum_type intent) {
		key_to_intent[id] = intent;
	}

	void map_event_to_intent(const message_type id, const intent_enum_type intent) {
		event_to_intent[id] = intent;
	}

	std::vector<basic_game_intent<intent_enum_type>> to_game_intents(const augs::machine_entropy::local_type& local) const {
		std::vector<basic_game_intent<intent_enum_type>> output;

		for (const auto& raw : local) {
			game_intent intent;

			bool found_context_entry = false;

			if (raw.was_any_key_pressed() || raw.was_any_key_released()) {
				intent.is_pressed = raw.was_any_key_pressed();

				const auto found_intent = key_to_intent.find(raw.key);
				if (found_intent != key_to_intent.end()) {
					intent.intent = (*found_intent).second;
					found_context_entry = true;
				}
			}
			else {
				intent.is_pressed = true;

				const auto found_intent = event_to_intent.find(raw.msg);
				if (found_intent != event_to_intent.end()) {
					intent.intent = (*found_intent).second;
					found_context_entry = true;
				}
			}

			intent.mouse_rel = raw.mouse.rel;

			if (found_context_entry) {
				output.push_back(intent);
			}
		}

		return output;
	}
};

