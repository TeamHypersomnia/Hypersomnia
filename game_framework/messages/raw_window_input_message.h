#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"

namespace messages {
	struct raw_window_input_message {
		augs::window::event::state raw_window_input;
	};
}