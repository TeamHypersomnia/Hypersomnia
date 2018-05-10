#pragma once
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"

namespace augs {
	struct atlas_entry {
		xywh atlas_space;
		bool was_flipped = false;
		pad_bytes<3> pad;

		/*
			Each atlas entry caches its original size 
			to speed up calculations during drawing,
			even though it could be calculated via multiplying atlas_space by the real atlas size.
		*/

		/*
			Initialize with some sensible size in case a glitch is inevitable
			for a fraction of a second.
		*/
		vec2u cached_original_size_pixels = vec2u(32, 32);

		vec2u get_original_size() const {
			return cached_original_size_pixels;
		}

		vec2 get_atlas_space_uv(const vec2 entry_space) const {
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

		bool exists() const {
			return atlas_space.w > 0.f;
		}
	};
}