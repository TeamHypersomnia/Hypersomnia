#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"

#include "game/enums/intent_type.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct unmapped_intent_message {
		intent_type intent;
		bool pressed_flag = false;
		bool delete_this_message = false;

		augs::window::event::state state;
	};
}