#pragma once
#include "augs/misc/enum/enum_array.h"

namespace augs {
	enum class dedicated_buffer {
		AVATARS,
		DEATH_SUMMARY_AVATAR,
		SPECTATOR_AVATAR,

		NICKNAMES,
		HEALTH_NUMBERS,
		INDICATORS,

		SCOREBOARD_COLOR_INDICATORS,

		FOG_OF_WAR,

		THUNDERS,
		EXPLODING_RINGS,

		GROUND_AND_DECORS,

		DIM_WANDERING_PIXELS,
		ILLUMINATING_WANDERING_PIXELS,

		INDICATORS_AND_CALLOUTS,
		SENTIENCE_INDICATORS,

		FLOOR_NEONS,
		FLOOR_NEON_OVERLAYS,
		BODY_NEONS,
		DECORATION_NEONS,

		SMALL_AND_GLASS_BODIES,
		WALL_LIGHTED_BODIES,
		OVER_SENTIENCES,
		CAPTIONS_AND_BULLETS,
		INSECTS,

		FRIENDLY_SENTIENCES,
		ENEMY_SENTIENCES,

		BORDERS_FRIENDLY_SENTIENCES,
		BORDERS_ENEMY_SENTIENCES,

		NEONS_FRIENDLY_SENTIENCES,
		NEONS_ENEMY_SENTIENCES,

		NEON_OCCLUDING_DYNAMIC_BODY,

		COUNT
	};

	enum class dedicated_buffer_vector {
		LIGHT_VISIBILITY,
		SENTIENCE_HUDS,
		EXPLOSIVE_HUDS,

		COUNT
	};

	struct triangles_and_specials {
		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		void clear() {
			triangles.clear();
			lines.clear();
			specials.clear();
		}
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

