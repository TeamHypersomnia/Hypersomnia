#pragma once
#include "game/container_sizes.h"
#include "game/assets/game_image_id.h"
#include "augs/drawing/polygon.h"

namespace components {
	using polygon = augs::polygon_with_id<
		assets::game_image_id,
		RENDERING_POLYGON_VERTEX_COUNT,
		RENDERING_POLYGON_INDEX_COUNT
	>;
}