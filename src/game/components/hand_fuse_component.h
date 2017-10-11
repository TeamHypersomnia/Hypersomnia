#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"

namespace components {
	struct hand_fuse {
		// GEN INTROSPECTOR struct components::hand_fuse
		augs::stepped_timestamp when_released;
		augs::stepped_timestamp when_detonates;

		sound_effect_input unpin_sound;
		sound_effect_input throw_sound;
		// END GEN INTROSPECTOR
	};
}
