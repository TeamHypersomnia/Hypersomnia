#pragma once
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

struct impact_effect_def {
	// GEN INTROSPECTOR struct impact_effect_def
	sound_effect_input sound;
	particle_effect_input particles;
	bool spawn_exploding_ring = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};

struct damage_effects {
	// GEN INTROSPECTOR struct damage_effects
	impact_effect_def impact;
	impact_effect_def sentience_impact;
	impact_effect_def destruction;
	// END GEN INTROSPECTOR
};

struct damage_definition {
	// GEN INTROSPECTOR struct damage_definition
	real32 base = 0.f;
	sentience_shake shake = { 400.f, 1.f };

	real32 impact_impulse = 10.f;
	real32 impulse_multiplier_against_sentience = 10.f;

	damage_effects effects;

	sound_effect_input pass_through_held_item_sound;
	// END GEN INTROSPECTOR

	template <class T>
	auto& operator*=(const T& scalar) {
		base *= scalar;
		shake *= scalar;
		impact_impulse *= scalar;
		return *this;
	}

	bool is_set() const {
		return base != 0.f;
	}
};

