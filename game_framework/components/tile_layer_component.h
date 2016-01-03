#pragma once
#include "math/vec2.h"
#include "game_framework/assets/texture.h"

namespace shared {
	class drawing_state;
}

namespace augs {
	class texture;

	struct tileset {
		struct tile_type {
			assets::texture_id tile_texture;
			tile_type(assets::texture_id = assets::texture_id::BLANK);
		};

		std::vector<tile_type> tile_types;
	};
}

namespace components {
	class tile_layer {
	public:
		struct tile {
			unsigned type_id = 0;

			tile(unsigned type);
		};

		tile_layer(augs::rects::wh<int> size);

		void generate_indices_by_type(augs::rects::ltrb<int>);

		void draw(shared::drawing_state&);

		augs::rects::ltrb<int> get_visible_tiles(shared::drawing_state&);

		augs::rects::ltrb<int> indices_by_type_visibility;
		std::vector<std::vector<vec2i>> indices_by_type;
		std::vector<tile> tiles;
		augs::rects::wh<int> size;
		int square_size = 32;
		augs::tileset* layer_tileset = nullptr;
	};
};
