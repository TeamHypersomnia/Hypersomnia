#pragma once
#include "message.h"
#include "math/vec2d.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augmentations;

namespace messages {
	struct intent_message : public message {
		enum class intent {
			MOVE_FORWARD,
			MOVE_BACKWARD,
			MOVE_LEFT,
			MOVE_RIGHT,
			SHOOT,
			AIM,
			SWITCH_LOOK,
			SWITCH_WEAPON
		};

		intent type;

		bool state_flag;
		vec2<int> mouse_pos, mouse_rel;

		intent_message(intent type, bool state_flag = true) : type(type), state_flag(state_flag) {}
		intent_message(intent type, vec2<int> mouse_pos, vec2<int> mouse_rel) : type(type), mouse_pos(mouse_pos), mouse_rel(mouse_rel), state_flag(true) {}
	};
}