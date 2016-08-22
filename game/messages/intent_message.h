#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "augs/window_framework/event.h"
#include "game/enums/intent_type.h"

struct entity_intent {
	intent_type intent;
	vec2i mouse_rel;
	bool pressed_flag = false;

	bool uses_mouse_motion() const {
		return
			intent == intent_type::MOVE_CROSSHAIR ||
			intent == intent_type::CROSSHAIR_PRIMARY_ACTION ||
			intent == intent_type::CROSSHAIR_SECONDARY_ACTION;
	}
};

namespace messages {
	struct intent_message : entity_intent, message {

	};
}

namespace augs {
	template<class A>
	void read_object(A& ar, entity_intent& intent) {
		read_object(ar, intent.intent);
		read_object(ar, intent.pressed_flag);

		if (intent.uses_mouse_motion())
			read_object(ar, intent.mouse_rel);
	}

	template<class A>
	void write_object(A& ar, const entity_intent& intent) {
		write_object(ar, intent.intent);
		write_object(ar, intent.pressed_flag);

		if (intent.uses_mouse_motion())
			write_object(ar, intent.mouse_rel);
	}
}