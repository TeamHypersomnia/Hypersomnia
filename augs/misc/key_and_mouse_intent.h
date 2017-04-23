#pragma once
#include "game/enums/intent_type.h"

#include "augs/window_framework/event.h"
#include "augs/padding_byte.h"

#include "augs/misc/container_with_small_size.h"

struct key_and_mouse_intent {
	intent_type intent = intent_type::COUNT;
	vec2t<short> mouse_rel;
	bool is_pressed = false;

	bool is_set() const;
	bool uses_mouse_motion() const;

	bool operator==(const key_and_mouse_intent& b) const;
	bool operator!=(const key_and_mouse_intent& b) const;
};

typedef augs::container_with_small_size<
	std::vector<key_and_mouse_intent>,
	unsigned char
> key_and_mouse_intent_vector;

namespace augs {
	template<class A>
	void read_object(A& ar, key_and_mouse_intent& intent) {
		read(ar, intent.intent);
		read(ar, intent.is_pressed);

		if (intent.uses_mouse_motion()) {
			read(ar, intent.mouse_rel);
		}
	}

	template<class A>
	void write_object(A& ar, const key_and_mouse_intent& intent) {
		write(ar, intent.intent);
		write(ar, intent.is_pressed);

		if (intent.uses_mouse_motion()) {
			write(ar, intent.mouse_rel);
		}
	}
}