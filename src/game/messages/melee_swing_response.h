#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"

namespace messages {
	struct melee_swing_response : message {
		transformr origin_transform;
	};
}
