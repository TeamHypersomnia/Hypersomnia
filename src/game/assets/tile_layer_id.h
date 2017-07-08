#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class tile_layer_id {
		INVALID,

#if BUILD_TEST_SCENES
		METROPOLIS_FLOOR,
		CATHEDRAL_FLOOR,
#endif

		COUNT = MAX_TILE_LAYER_COUNT + 1
	};
}