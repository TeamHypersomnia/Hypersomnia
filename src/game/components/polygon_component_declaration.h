#pragma once
#include <cstddef>
#include "game/assets/ids/asset_ids.h"
#include "game/container_sizes.h"

namespace augs {
	template <class id_type, std::size_t vertex_count, std::size_t index_count>
	struct polygon_with_id;
}

namespace invariants {
	using polygon = augs::polygon_with_id<
		assets::image_id,
		RENDERING_POLYGON_VERTEX_COUNT,
		RENDERING_POLYGON_INDEX_COUNT
	>;
}