#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/assets/ids/asset_ids.h"

static constexpr std::size_t MAX_STAR_FIELD_STAR_TYPES = 16;

struct star_field_star_type {
	// GEN INTROSPECTOR struct star_field_star_type
	assets::image_id image_id;
	real32 weight = 1.f;
	// END GEN INTROSPECTOR

	bool operator==(const star_field_star_type&) const = default;
};

namespace invariants {
	struct star_field {
		// GEN INTROSPECTOR struct invariants::star_field
		augs::constant_size_vector<star_field_star_type, MAX_STAR_FIELD_STAR_TYPES> star_types;
		real32 stars_per_million_square_units = 100.f;
		uint32_t seed = 0;
		real32 min_scale = 1.f;
		real32 max_scale = 1.f;
		real32 min_brightness = 1.f;
		// END GEN INTROSPECTOR

		bool is_enabled() const {
			return !star_types.empty();
		}
	};
}
