#pragma once
#include "message.h"
#include "../components/transform_component.h"

namespace messages {
	struct melee_swing_response : message {
		components::transform origin_transform;
	};
}
