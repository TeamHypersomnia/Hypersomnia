#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "intent_message.h"

namespace messages {
	struct crosshair_intent_message : public intent_message {
		vec2 crosshair_base_offset;
		vec2 crosshair_base_offset_rel;

		vec2 crosshair_world_pos;
	};
}