#pragma once
#include <vector>
#include <cstdint>
#include "augs/math/declare_math.h"
#include "game/components/star_field_component.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_sprite_resource;

struct editor_star_field_star {
	// GEN INTROSPECTOR struct editor_star_field_star
	editor_typed_resource_id<editor_sprite_resource> sprite;
	real32 weight = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_star_field_star&) const = default;
};

struct editor_star_field {
	// GEN INTROSPECTOR struct editor_star_field
	std::vector<editor_star_field_star> stars;
	real32 stars_per_million_square_units = 100.0f;
	uint32_t seed = 0;
	real32 min_scale = 1.0f;
	real32 max_scale = 1.0f;
	real32 min_brightness = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_star_field&) const = default;
};

template <class F>
invariants::star_field to_game_star_field(const editor_star_field& from, F find_resource) {
	auto result = invariants::star_field();

	for (const auto& star : from.stars) {
		if (result.star_types.size() == result.star_types.max_size()) {
			break;
		}

		if (const auto sprite_resource = find_resource(star.sprite)) {
			result.star_types.push_back({ sprite_resource->scene_asset_id, star.weight });
		}
	}

	result.stars_per_million_square_units = from.stars_per_million_square_units;
	result.seed = from.seed;
	result.min_scale = from.min_scale;
	result.max_scale = from.max_scale;
	result.min_brightness = from.min_brightness;

	return result;
}
