#pragma once
#include "game/components/sprite_component.h"
#include "augs/misc/constant_size_vector.h"

namespace components {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct components::wandering_pixels
		vec2 center;
		vec2 size;
		rgba colorize = white;
		unsigned particles_count = 0u;
		// END GEN INTROSPECTOR

		void set_reach(const xywh a) {
			center = a.get_center();
			size = a.get_size();
		}

		auto get_reach() const {
			return xywh::center_and_size(center, size);
		}
	};
}

using wandering_pixels_frames = augs::constant_size_vector<invariants::sprite, 10>;

namespace invariants {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct invariants::wandering_pixels
		wandering_pixels_frames frames = {};
		float frame_duration_ms = 300.f;
		// END GEN INTROSPECTOR

		const invariants::sprite& get_face_after(const float passed_lifetime_ms) const {
			const auto frame_count = frames.size();
			const auto frame_num = static_cast<unsigned>(passed_lifetime_ms / frame_duration_ms) % frame_count;

			return frames[frame_num];
		}
	};
}