#pragma once
#include "augs/padding_byte.h"

namespace augs {
	struct sound_effect_modifier {
		int repetitions = 1;
		float gain = 1.f;
		float pitch = 1.f;
		float max_distance = 1920.f * 5.f;
		float reference_distance = 0.f;
	};
}