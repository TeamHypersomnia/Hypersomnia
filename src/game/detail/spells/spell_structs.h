#pragma once
#include <string>
#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/asset_ids.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

#include "game/common_state/entity_name_str.h"

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
	assets::image_id icon;
	entity_name_str incantation;
	entity_name_str name;
	entity_name_str description;
	// END GEN INTROSPECTOR
};