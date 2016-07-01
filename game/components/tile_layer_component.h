#pragma once
#include "math/vec2.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"

namespace shared {
	struct state_for_drawing_renderable;
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
	struct tile_layer {
		struct tile {
			unsigned type_id = 0;

			tile(unsigned type);
		};

		tile_layer(augs::rects::wh<int> size = augs::rects::wh<int>());

		void generate_indices_by_type(augs::rects::ltrb<int>);

		void draw(const shared::state_for_drawing_renderable&) const;
		augs::rects::ltrb<float> get_aabb(components::transform transform) const;

		augs::rects::ltrb<int> get_visible_tiles(const shared::state_for_drawing_renderable&) const;

		augs::rects::ltrb<int> indices_by_type_visibility;
		std::vector<std::vector<vec2i>> indices_by_type;
		std::vector<tile> tiles;
		augs::rects::wh<int> size;
		int square_size = 32;
		augs::tileset* layer_tileset = nullptr;
	};
};
