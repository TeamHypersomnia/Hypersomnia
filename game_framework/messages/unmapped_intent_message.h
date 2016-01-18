#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"
#include "utilities/misc/constant_vector.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct unmapped_intent_message {
		enum intent_type {
			NONE,
			
			PRESS_TRIGGER,
			RELEASE_CAR,

			MOVE_FORWARD,
			MOVE_BACKWARD,
			MOVE_LEFT,
			MOVE_RIGHT,
			HAND_BRAKE,
			SHOOT,
			MOVE_CROSSHAIR,
			CROSSHAIR_PRIMARY_ACTION,
			CROSSHAIR_SECONDARY_ACTION,
			SWITCH_LOOK,
			ZOOM_CAMERA,
			SWITCH_WEAPON
		};

		struct intent_set {
			augs::constant_vector<intent_type, 5> intents;

			bool operator==(intent_type it) const {
				return std::find(intents.begin(), intents.end(), it) != intents.end();
			}			
			
			bool operator!=(intent_type it) const {
				return std::find(intents.begin(), intents.end(), it) == intents.end();
			}
		};

		void clear() {
			intent.intents.clear();
		}

		intent_set intent;
		bool pressed_flag = false;

		augs::window::event::state state;
	};
}