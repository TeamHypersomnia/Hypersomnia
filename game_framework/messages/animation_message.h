#pragma once
#include "message.h"
#include "../assets/animation.h"

namespace messages {
	struct change_animation_state : message {
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
	};

	struct animation_message : change_animation_state {
		/* if nullptr, will take animation_type as key for animation_subscribtion map in animation_component */
		assets::animation_id set_animation;
	};
}
