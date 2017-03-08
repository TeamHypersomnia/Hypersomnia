#pragma once
#include "game/enums/intent_type.h"
#include "augs/window_framework/event.h"
#include "augs/padding_byte.h"

struct key_and_mouse_intent {
	intent_type intent = intent_type::NONE;
	vec2t<short> mouse_rel;
	bool is_pressed = false;

	bool uses_mouse_motion() const;

	bool operator==(const key_and_mouse_intent& b) const;
	bool operator!=(const key_and_mouse_intent& b) const;
};

bool operator==(const std::vector<key_and_mouse_intent>& a, const std::vector<key_and_mouse_intent>& b);
bool operator!=(const std::vector<key_and_mouse_intent>& a, const std::vector<key_and_mouse_intent>& b);

namespace augs {
	template<class A>
	void read_object(A& ar, key_and_mouse_intent& intent) {
		if (!read_object(ar, intent.intent)) return false;
		if (!read_object(ar, intent.is_pressed)) return false;

		if (intent.uses_mouse_motion()) {
			if (!read_object(ar, intent.mouse_rel)) {
				return false;
			}
		}

		return true;
	}

	template<class A>
	void write_object(A& ar, const key_and_mouse_intent& intent) {
		write_object(ar, intent.intent);
		write_object(ar, intent.is_pressed);

		if (intent.uses_mouse_motion()) {
			write_object(ar, intent.mouse_rel);
		}
	}
}