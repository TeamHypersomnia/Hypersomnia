#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/drawing/flip.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "game/assets/ids/game_image_id.h"

struct game_image_usage_as_button {
	// GEN INTROSPECTOR struct game_image_usage_as_button
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct game_image_meta {
	// GEN INTROSPECTOR struct game_image_meta
	game_image_usage_as_button usage_as_button;
	std::optional<std::vector<vec2u>> physical_shape;
	// END GEN INTROSPECTOR
};

struct game_image_in_atlas {
	augs::texture_atlas_entry diffuse;
	augs::texture_atlas_entry neon_map;
	augs::texture_atlas_entry desaturated;

	operator augs::texture_atlas_entry() const {
		return diffuse;
	}

	vec2u get_size() const {
		return diffuse.get_original_size();
	}
};