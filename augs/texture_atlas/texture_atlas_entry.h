#pragma once
#include "augs/math/vec2.h"

namespace augs {
	struct texture_atlas_entry {
		xywh atlas_space;
		bool was_flipped = false;
		
		vec2 get_atlas_space_uv(const vec2 entry_space) const;
		bool exists() const;
	};
}