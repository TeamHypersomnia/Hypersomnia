#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "augs/window_framework/event.h"
#include "game/enums/intent_type.h"

struct entity_intent {
	intent_type intent;
	vec2i mouse_rel;
	bool pressed_flag = false;
};

namespace messages {
	struct intent_message : entity_intent, message {

	};
}