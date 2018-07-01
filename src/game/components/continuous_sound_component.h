#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/view_input/sound_effect_input.h"

namespace invariants {
	struct continuous_sound {
		// GEN INTROSPECTOR struct invariants::continuous_sound
		sound_effect_input effect;
		// END GEN INTROSPECTOR

		continuous_sound() {
			effect.modifier.repetitions = -1;
			effect.modifier.reference_distance = 150.f;
		}
	};
}
