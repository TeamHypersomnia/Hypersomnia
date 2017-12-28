#pragma once
#include "augs/pad_bytes.h"
#include "game/assets/ids/sound_buffer_id.h"

struct sound_effect_modifier {
	// GEN INTROSPECTOR struct sound_effect_modifier
	float gain = 1.f;
	float pitch = 1.f;
	float max_distance = 1920.f * 3.f;
	float reference_distance = 0.f;
	short repetitions = 1;
	bool fade_on_exit = true;
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR
};

struct sound_effect_input {
	// GEN INTROSPECTOR struct sound_effect_input
	assets::sound_buffer_id id = assets::sound_buffer_id::INVALID;
	sound_effect_modifier modifier;
	// END GEN INTROSPECTOR
};