#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"

namespace messages {
	struct interpolation_correction_request : message {
		entity_id set_previous_transform_from;
		transformr set_previous_transform_value;
	};
}