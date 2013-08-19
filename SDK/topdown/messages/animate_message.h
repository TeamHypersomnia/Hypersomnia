#pragma once
#include "message.h"
#include "utility/sorted_vector.h"

namespace messages {
	struct animate_message : public message {
		enum class animation {
			MOVE
		} animation_type;

		enum class type {
			CONTINUE,
			START,
			PAUSE,
			STOP
		} message_type;

		bool preserve_state;
		bool change_animation;
		bool change_speed;
		float speed_factor;

		animate_message() : change_speed(false), speed_factor(1.f), change_animation(false), preserve_state(false) {}
		animate_message(animation animation_type, type message_type, bool override_speed, float speed) 
			: animation_type(animation_type), message_type(message_type), change_speed(change_speed), speed_factor(speed_factor)
		{}
	};
}
