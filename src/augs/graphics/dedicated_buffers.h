#pragma once
#include "augs/misc/enum/enum_array.h"

namespace augs {
	enum class dedicated_buffer {
		AVATARS,
		DEATH_SUMMARY_AVATAR,

		NICKNAMES,
		HEALTH_NUMBERS,
		INDICATORS,

		SCOREBOARD_COLOR_INDICATORS,

		FOG_OF_WAR,

		THUNDERS,
		EXPLOSION_RINGS,

		COUNT
	};

	enum class dedicated_buffer_vector {
		LIGHT_VISIBILITY,

		COUNT
	};

	struct triangles_and_specials {
		vertex_triangle_buffer triangles;
		special_buffer specials;
	};

	struct dedicated_buffers {
		enum_array<triangles_and_specials, dedicated_buffer> single;
		enum_array<std::vector<triangles_and_specials>, dedicated_buffer_vector> vectors;

		auto& operator[](const dedicated_buffer d) {
			return single[d];
		}

		auto& operator[](const dedicated_buffer_vector d) {
			return vectors[d];
		}
		
		auto& operator[](const dedicated_buffer d) const {
			return single[d];
		}

		auto& operator[](const dedicated_buffer_vector d) const {
			return vectors[d];
		}
	};
}

