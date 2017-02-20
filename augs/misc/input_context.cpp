#include "input_context.h"
#include "augs/misc/machine_entropy.h"

using namespace augs::window::event;
using namespace keys;

template <class T>
key basic_input_context<T>::get_bound_key_if_any(const T intent) const {
	for (const auto& k : key_to_intent) {
		if (k.second == intent) {
			return k.first;
		}
	}

	return key::INVALID;
}

template <class T>
void basic_input_context<T>::map_key_to_intent(const key id, const T intent) {
	key_to_intent[id] = intent;
}

template <class T>
void basic_input_context<T>::map_event_to_intent(const message id, const T intent) {
	event_to_intent[id] = intent;
}

template <class T>
std::vector<key_and_mouse_intent> basic_input_context<T>::to_key_and_mouse_intents(const augs::machine_entropy::local_type& local) const {
	std::vector<key_and_mouse_intent> output;

	for (const auto& raw : local) {
		key_and_mouse_intent intent;

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

	return std::move(output);
}

template struct basic_input_context<intent_type>;