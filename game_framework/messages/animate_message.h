#pragma once
#include "message.h"

namespace resources {
	struct animation;
}

namespace messages {
	struct animate_message : public message {
		enum animation {
			MOVE,
			SHOT,
			SWING_CW,
			SWING_CCW,
			MOVE_CW,
			MOVE_CCW
		};


		enum type {
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

		/* if nullptr, will take animation_type as key for animation_subscribtion map in animate_component */
		resources::animation* set_animation = nullptr;

		animate_message() {}
			animate_message(animation animation_type, type message_type, bool override_speed, float speed)
			: animation_type(animation_type), message_type(message_type), change_speed(change_speed), speed_factor(speed_factor), animation_priority(0)
		{}
	};
}
