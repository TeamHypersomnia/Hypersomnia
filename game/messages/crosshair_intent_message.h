#pragma once
#include "message.h"
#include "augs/math/vec2.h"

#include "intent_message.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct crosshair_intent_message : public intent_message {
		vec2 crosshair_base_offset;
		vec2 crosshair_base_offset_rel;

		vec2 crosshair_world_pos;
	};
}