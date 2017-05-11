#pragma once
#include "game/container_sizes.h"

namespace assets {
	enum class tile_layer_id {
		INVALID,

		METROPOLIS_FLOOR,
		CATHEDRAL_FLOOR,

		COUNT = MAX_TILE_LAYER_COUNT
	};
}