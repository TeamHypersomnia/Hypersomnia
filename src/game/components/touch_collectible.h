#pragma once
#include "game/detail/sentience/pe_absorption.h"
#include "game/detail/economy/money_type.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace components {

}

/* TODO: Make a trivial variant out of this. */

enum class touch_collectible_type {
	// GEN INTROSPECTOR enum class touch_collectible_type
	COIN,

	COUNT
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct touch_collectible {
		// GEN INTROSPECTOR struct invariants::touch_collectible
		money_type money_value = 100;
		sound_effect_input collect_sound;
		particle_effect_input collect_particles;
		rgba associated_col = white;
		// END GEN INTROSPECTOR
	};
}