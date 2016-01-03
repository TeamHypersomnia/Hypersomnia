#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct intent_message : public message {
		enum intent_type {
			MOVE_FORWARD,
			MOVE_BACKWARD,
			MOVE_LEFT,
			MOVE_RIGHT,
			SHOOT,
			AIM,
			SWITCH_LOOK,
			ZOOM_CAMERA,
			SWITCH_WEAPON
		};

		unsigned intent = 0;
		bool pressed_flag = false;

		augs::window::event::state state;
	};
}