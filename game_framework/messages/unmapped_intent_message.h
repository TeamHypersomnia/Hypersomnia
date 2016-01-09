#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct unmapped_intent_message {
		enum intent_type {
			NONE,
			MOVE_FORWARD,
			MOVE_BACKWARD,
			MOVE_LEFT,
			MOVE_RIGHT,
			SHOOT,
			MOVE_CROSSHAIR,
			CROSSHAIR_PRIMARY_ACTION,
			CROSSHAIR_SECONDARY_ACTION,
			SWITCH_LOOK,
			ZOOM_CAMERA,
			SWITCH_WEAPON
		};

		intent_type intent = intent_type::NONE;
		bool pressed_flag = false;

		augs::window::event::state state;
	};
}