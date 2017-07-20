#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class shader_program_id {
		// GEN INTROSPECTOR enum class assets::shader_program_id
		INVALID,
		
		DEFAULT,
		DEFAULT_ILLUMINATED,
		LIGHT,
		PURE_COLOR_HIGHLIGHT,
		CIRCULAR_BARS,
		EXPLODING_RING,
		SMOKE,
		ILLUMINATING_SMOKE,
		SPECULAR_HIGHLIGHTS,

		REQUISITE_COUNT,
		COUNT = MAX_SHADER_PROGRAM_COUNT + 1
		// END GEN INTROSPECTOR
	};
}