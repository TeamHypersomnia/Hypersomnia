#pragma once
#include "message.h"
#include "math/vec2d.h"

namespace messages {
	struct collision_message : public message {
		augmentations::entity_system::entity* collider;
		augmentations::vec2<> position;
	};
}