#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class program_id {
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
		FULLSCREEN,
		COUNT = MAX_SHADER_PROGRAM_COUNT
	};
}