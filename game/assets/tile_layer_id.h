#pragma once

namespace resources {
	class tile_layer;
}

namespace assets {
	enum class tile_layer_id {
		INVALID,

		METROPOLIS_FLOOR,
		CATHEDRAL_FLOOR,

		COUNT
	};
}

resources::tile_layer& operator*(const assets::tile_layer_id& id);
bool operator!(const assets::tile_layer_id& id);