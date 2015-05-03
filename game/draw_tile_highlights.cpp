#include "stdafx.h"
#include "utilities.h"
void draw_tile_highlights(
	resources::tile_layer& tile_layer, 
	resources::animation& blink_animation, 
	std::vector<int>& shining,
	resources::renderable::draw_input& in) {
}

vec2<int> get_random_coordinate_on_a_special_tile(
	resources::tile_layer& tile_layer,
	augs::misc::vector_wrapper<int>& shining,
	resources::renderable::draw_input& in) {

	auto visible_tiles = tile_layer.get_visible_tiles(in);

	auto draw_input_copy = in;
	auto& tiles = tile_layer.tiles;

	std::vector<vec2<int>> all_shining_indices;

	for (int y = visible_tiles.t; y < visible_tiles.b; ++y) {
		for (int x = visible_tiles.l; x < visible_tiles.r; ++x) {
			auto tile_offset = vec2<int>(x, y) * tile_layer.square_size;

			int idx = y * tile_layer.size.w + x;

			bool found_shining = false;

			for (int k = 0; k < shining.size(); ++k) {
				if (tiles[idx].type_id == shining.at(k)) {
					found_shining = true;
					all_shining_indices.push_back(tile_offset);
					break;
				}
			}

		}
	}

	return all_shining_indices.empty() ? vec2<int>(-1, -1) : all_shining_indices[randval(int(0), int(all_shining_indices.size() - 1))];
}