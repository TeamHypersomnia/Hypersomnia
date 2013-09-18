#pragma once
#include "message.h"
#include "math/vec2d.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augmentations;

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
			SWITCH_WEAPON
		};

		unsigned intent;

		bool state_flag;
		vec2<int> mouse_pos, mouse_rel;

		intent_message(unsigned intent = 0, bool state_flag = true) : intent(intent), state_flag(state_flag) {}
		intent_message(unsigned intent, vec2<int> mouse_pos, vec2<int> mouse_rel) : intent(intent), mouse_pos(mouse_pos), mouse_rel(mouse_rel), state_flag(true) {}
	};
}