#pragma once
#include "message.h"
#include "math/vec2.h"

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
			SWITCH_WEAPON
		};

		unsigned intent;

		bool state_flag;
		vec2<> mouse_pos, mouse_rel;

		int wheel_amount;

		intent_message(unsigned intent = 0, bool state_flag = true) : intent(intent), state_flag(state_flag), wheel_amount(0) { send_to_scripts = true; }
		intent_message(unsigned intent, vec2<> mouse_pos, vec2<> mouse_rel) : intent(intent), mouse_pos(mouse_pos), mouse_rel(mouse_rel), state_flag(true), wheel_amount(0) { send_to_scripts = true; }
	};
}