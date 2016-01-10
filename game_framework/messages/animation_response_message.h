#pragma once
#include "message.h"
#include "animation_message.h"

namespace messages {
	struct animation_response_message : change_animation_state {
		enum response {
			MOVE,
			SHOT,
			SWING_CW,
			SWING_CCW,
			MOVE_CW,
			MOVE_CCW
		};

		response response_type;
	};
}
