#pragma once
#include "augs/window_framework/event.h"

#include "augs/misc/enum_associative_array.h"
#include "augs/misc/basic_game_intent.h"
#include "augs/misc/basic_game_motion.h"

#include "augs/misc/machine_entropy.h"

template <class intent_enum_type, class motion_enum_type>
struct basic_input_context {
	typedef augs::event::keys::key key_type;
	typedef augs::event::message message_type;

	// GEN INTROSPECTOR struct basic_input_context class intent_enum_type class motion_enum_type
	augs::enum_associative_array<key_type, intent_enum_type> key_to_intent;
	motion_enum_type map_mouse_motion_to = motion_enum_type::INVALID;
	// END GEN INTROSPECTOR

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

	struct translated {
		std::vector<basic_game_intent<intent_enum_type>> intents;
		std::vector<basic_game_motion<motion_enum_type>> motions;
	};

	auto translate(const augs::machine_entropy::local_type& local) const {
		translated output;

		for (const auto& raw : local) {
			if (raw.was_any_key_pressed() || raw.was_any_key_released()) {
				const auto found_intent = key_to_intent.find(raw.key);

				if (found_intent != key_to_intent.end()) {
					game_intent intent;

					intent.is_pressed = raw.was_any_key_pressed();
					intent.intent = (*found_intent).second;

					output.intents.push_back(intent);
				}
			}
			else if (raw.msg == augs::event::message::mousemotion) {
				basic_game_motion<motion_enum_type> motion;
				motion.set_motion_type(map_mouse_motion_to);
				motion.offset = raw.mouse.rel;
				output.motions.push_back(motion);
			}
		}

		return output;
	}
};

