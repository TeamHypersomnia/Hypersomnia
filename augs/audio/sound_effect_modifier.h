#pragma once
#include "augs/padding_byte.h"

namespace augs {
	struct sound_effect_modifier {
		// GEN INTROSPECTOR struct augs::sound_effect_modifier
		int repetitions = 1;
		float gain = 1.f;
		float pitch = 1.f;
		float max_distance = 1920.f * 3.f;
		float reference_distance = 0.f;
		bool fade_on_exit = true;
		std::array<padding_byte, 3> pad;
		// END GEN INTROSPECTOR
	};
}