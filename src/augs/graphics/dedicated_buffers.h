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
		SMALL_HEALTH_BARS,

		SCOREBOARD_COLOR_INDICATORS,

		FOG_OF_WAR,

		THUNDERS,
		EXPLODING_RINGS,

		GROUND,

		DIM_WANDERING_PIXELS,
		ILLUMINATING_WANDERING_PIXELS,

		INDICATORS_AND_CALLOUTS,
		SENTIENCE_INDICATORS,

		GROUND_NEONS,
		GROUND_NEON_OCCLUDERS,
		FOREGROUND_NEON_OCCLUDERS,

		DROPPED_ITEMS_SHADOWS,
		DROPPED_ITEMS_BORDERS,
		DROPPED_ITEMS_DIFFUSE,
		DROPPED_ITEMS_NEONS,
		DROPPED_ITEMS_OVERLAYS,

		REMNANTS,
		SOLID_OBSTACLES,

		WALL_ILLUMINATIONS,

		FOREGROUND,
		FOREGROUND_GLOWS,
		UNDER_FOREGROUND_NEONS,
		FOREGROUND_NEONS,

		FRIENDLY_SENTIENCES,
		ENEMY_SENTIENCES,

		BORDERS_FRIENDLY_SENTIENCES,
		BORDERS_ENEMY_SENTIENCES,

		NEONS_FRIENDLY_SENTIENCES,
		NEONS_ENEMY_SENTIENCES,

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

