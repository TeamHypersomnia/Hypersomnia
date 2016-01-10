#pragma once
#include "message.h"
#include "animation_message.h"

namespace messages {
	struct animation_response_message : change_animation_state {
		enum response_type {
			MOVE,
			SHOT,
			SWING_CW,
			SWING_CCW,
			MOVE_CW,
			MOVE_CCW
		};

		response_type response;
	};
}
