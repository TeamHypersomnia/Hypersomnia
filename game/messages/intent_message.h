#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"
#include "game/enums/intent_type.h"

namespace messages {
	struct intent_message : public message {
		intent_type intent;
		bool pressed_flag = false;
		bool delete_this_message = false;

		augs::window::event::state state;
	};
}