#pragma once
#include "game/messages/message.h"
#include "game/assets/ids/asset_ids.h"

namespace messages {
	struct animation_message : message {
		enum action_type {
			INVALID,
			CONTINUE,
			START,
			PAUSE,
			STOP
		};

		action_type action = INVALID;

		assets::animation_id set_animation;
		bool preserve_state_if_animation_changes = false;
		bool change_animation = false;
		bool change_speed = false;
		float speed_factor = 1.f;
		int animation_priority = 0;
	};
}
