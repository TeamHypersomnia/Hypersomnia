#pragma once
#include "augs/texture_atlas/atlas_entry.h"

struct image_in_atlas {
	augs::atlas_entry diffuse;
	augs::atlas_entry neon_map;
	augs::atlas_entry desaturated;

	explicit operator augs::atlas_entry() const {
		return diffuse;
	}

	vec2u get_original_size() const {
		return diffuse.get_original_size();
	}
};
