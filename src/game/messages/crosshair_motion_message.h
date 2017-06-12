#pragma once
#include "message.h"
#include "augs/math/vec2.h"

namespace messages {
	struct crosshair_motion_message : message {
		vec2 crosshair_base_offset;
		vec2 crosshair_base_offset_rel;

		vec2 crosshair_world_pos;
	};
}