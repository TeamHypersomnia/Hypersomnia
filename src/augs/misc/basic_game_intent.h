#pragma once
#include "augs/math/vec2.h"

template <class intent_type_enum>
struct basic_game_intent {
	intent_type_enum intent = intent_type_enum::INVALID;
	vec2t<short> mouse_rel;
	bool is_pressed = false;

	bool is_set() const {
		return intent != intent_type_enum::INVALID;
	}

	bool uses_mouse_motion() const {
		return
			intent == intent_type_enum::MOVE_CROSSHAIR
			|| intent == intent_type_enum::CROSSHAIR_PRIMARY_ACTION
			|| intent == intent_type_enum::CROSSHAIR_SECONDARY_ACTION
		;
	}

	bool operator==(const basic_game_intent& b) const {
		return
			intent == b.intent
			&& mouse_rel == b.mouse_rel
			&& is_pressed == b.is_pressed
		;
	}

	bool operator!=(const basic_game_intent& b) const {
		return !operator==(b);
	}
};

namespace augs {
	template <class A, class E>
	void read_object(A& ar, basic_game_intent<E>& intent) {
		read(ar, intent.intent);
		read(ar, intent.is_pressed);

		if (intent.uses_mouse_motion()) {
			read(ar, intent.mouse_rel);
		}
	}

	template <class A, class E>
	void write_object(A& ar, const basic_game_intent<E>& intent) {
		write(ar, intent.intent);
		write(ar, intent.is_pressed);

		if (intent.uses_mouse_motion()) {
			write(ar, intent.mouse_rel);
		}
	}
}