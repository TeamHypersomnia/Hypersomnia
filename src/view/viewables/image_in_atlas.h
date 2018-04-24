#pragma once
#include "augs/texture_atlas/texture_atlas_entry.h"

struct image_in_atlas {
	augs::texture_atlas_entry diffuse;
	augs::texture_atlas_entry neon_map;
	augs::texture_atlas_entry desaturated;

	operator augs::texture_atlas_entry() const {
		return diffuse;
	}

	vec2u get_original_size() const {
		return diffuse.get_original_size();
	}
};
