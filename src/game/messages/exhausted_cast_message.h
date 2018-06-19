#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"

namespace messages {
	struct exhausted_cast : message {
		transformr transform;
	};
}