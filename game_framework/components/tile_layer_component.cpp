#include "tile_layer_component.h"

#include "sprite_component.h"

#include "../shared/drawing_state.h"

#include "graphics/vertex.h"

using namespace components;
using namespace augs;
using namespace shared;

namespace augs {
	tileset::tile_type::tile_type(assets::texture_id tile_texture) : tile_texture(tile_texture) {

	}
}

namespace components {
	tile_layer::tile_layer(rects::wh<int> size) : size(size) {
		tiles.reserve(size.area());
	}

	tile_layer::tile::tile(unsigned type) : type_id(type) {}


	rects::ltrb<int> tile_layer::get_visible_tiles(drawing_state& in) {
		rects::ltrb<int> visible_tiles;
		
		visible_tiles.l = int((in.transformed_visible_world_area_aabb.l - in.object_transform.pos.x) / 32.f);
		visible_tiles.t = int((in.transformed_visible_world_area_aabb.t - in.object_transform.pos.y) / 32.f);
		visible_tiles.r = int((in.transformed_visible_world_area_aabb.r - in.object_transform.pos.x) / 32.f) + 1;
		visible_tiles.b = int((in.transformed_visible_world_area_aabb.b - in.object_transform.pos.y) / 32.f) + 1;
		visible_tiles.l = std::max(0, visible_tiles.l);
		visible_tiles.t = std::max(0, visible_tiles.t);
		visible_tiles.r = std::min(size.w, visible_tiles.r);
		visible_tiles.b = std::min(size.h, visible_tiles.b);

		return visible_tiles;
	}

	void tile_layer::draw(drawing_state& in) {
		/* if it is not visible, return */
		if (!in.transformed_visible_world_area_aabb.hover(rects::xywh<float>(in.object_transform.pos.x, in.object_transform.pos.y, size.w*square_size, size.h*square_size))) return;

		auto visible_tiles = get_visible_tiles(in);

		drawing_state draw_input_copy = in;

		for (int y = visible_tiles.t; y < visible_tiles.b; ++y) {
			for (int x = visible_tiles.l; x < visible_tiles.r; ++x) {
				vertex_triangle t1, t2;

				auto tile_offset = vec2i(x, y) * square_size;

				int idx = y * size.w + x;

				if (tiles[idx].type_id == 0) continue;

				auto& type = layer_tileset->tile_types[tiles[idx].type_id - 1];

				static thread_local sprite tile_sprite;
				tile_sprite.tex = type.tile_texture;

				draw_input_copy.object_transform.pos = vec2i(in.object_transform.pos) + tile_offset + vec2(square_size / 2, square_size / 2);

				tile_sprite.draw(draw_input_copy);
			}
		}
	}

	void tile_layer::generate_indices_by_type(rects::ltrb<int> visible_tiles) {
		if (visible_tiles == indices_by_type_visibility)
			return;

		indices_by_type_visibility = visible_tiles;

		for (auto& index_vector : indices_by_type)
			index_vector.clear();

		for (int y = visible_tiles.t; y < visible_tiles.b; ++y) {
			for (int x = visible_tiles.l; x < visible_tiles.r; ++x) {
				int i = y * size.w + x;

				auto type = tiles[i].type_id;
				if (type == 0) continue;

				if (indices_by_type.size() < type + 1)
					indices_by_type.resize(type + 1);

				indices_by_type[type].push_back(vec2i(x, y) * square_size);
			}
		}
	}

	rects::ltrb<float> tile_layer::get_aabb(components::transform transform) {
		return rects::xywh<float>(transform.pos.x, transform.pos.y, size.w*square_size, size.h*square_size);
	}
}
