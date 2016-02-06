#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"
#include "augs/misc/constant_size_vector.h"

#include "../globals/intents.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct unmapped_intent_message {

		struct intent_set {
			augs::constant_size_vector<intent_type, 5> intents;

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