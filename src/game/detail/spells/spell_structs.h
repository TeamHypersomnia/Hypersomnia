#pragma once
#include <string>
#include "augs/graphics/rgba.h"

#include "game/assets/ids/game_image_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/assets/ids/sound_buffer_id.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

struct spell_common_data {
	// GEN INTROSPECTOR struct spell_common_data
	particle_effect_input cast_sparkles;
	sound_effect_input cast_successful_sound;

	unsigned personal_electricity_required = 40u;
	unsigned cooldown_ms = 5000u;
	rgba associated_color;
	// END GEN INTROSPECTOR
};

struct spell_appearance {
	// GEN INTROSPECTOR struct spell_appearance
	assets::game_image_id icon = assets::game_image_id::INVALID;
	std::wstring incantation;
	std::wstring name;
	std::wstring description;
	// END GEN INTROSPECTOR
};