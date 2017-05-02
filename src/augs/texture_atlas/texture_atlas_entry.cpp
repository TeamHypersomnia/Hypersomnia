#include "texture_atlas_entry.h"

namespace augs {
	bool texture_atlas_entry::exists() const {
		return atlas_space.w > 0.f;
	}

	vec2 texture_atlas_entry::get_atlas_space_uv(vec2 entry_space) const {
		if (!was_flipped) {
			return { 
				atlas_space.x + atlas_space.w * entry_space.x,
				atlas_space.y + atlas_space.h * entry_space.y
			};
		}
		else {
			return {
				atlas_space.x + atlas_space.w * entry_space.y,
				atlas_space.y + atlas_space.h * entry_space.x
			};
		}
	}
}