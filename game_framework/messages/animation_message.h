#pragma once
#include "message.h"
#include "../resources/animation.h"

namespace resources {
	struct animation;
}

namespace messages {
	struct animation_message : public message {
		enum response {
			MOVE,
			SHOT,
			SWING_CW,
			SWING_CCW,
			MOVE_CW,
			MOVE_CCW
		};

		enum action {
			CONTINUE,
			START,
			PAUSE,
			STOP
		};
		
		int message_type = 0;
		int animation_type = 0;

		bool preserve_state_if_animation_changes = false;
		bool change_animation = false;
		bool change_speed = false;
		float speed_factor = 1.f;
		int animation_priority = 0;

		/* if nullptr, will take animation_type as key for animation_subscribtion map in animation_component */
		resources::animation* set_animation = nullptr;
	};
}
