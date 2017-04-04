#pragma once
#include "message.h"

namespace messages {
	struct movement_event : message {
		float speed = -1.f;
		bool stop_response_at_zero_speed;
	};
}
