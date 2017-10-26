#pragma once
#include <cstddef>
#include "game/assets/ids/game_image_id.h"

namespace augs {
	template <class id_type, std::size_t vertex_count, std::size_t index_count>
	struct polygon_with_id;
}

namespace components {
	using polygon = augs::polygon_with_id<
		assets::game_image_id,
		RENDERING_POLYGON_VERTEX_COUNT,
		RENDERING_POLYGON_INDEX_COUNT
	>;
}