#pragma once
#include "message.h"
#include "game/components/transform_component.h"

namespace messages {
	struct exhausted_cast : message {
		components::transform transform;
	};
}