#include "math/vec2.h"
#include "utilities.h"

void draw_tile_highlights(
	resources::tile_layer& tile_layer, 
	resources::animation& blink_animation, 
	std::vector<int>& shining,
	resources::renderable::draw_input& in) {
}

vec2i get_random_coordinate_on_a_special_tile(
	resources::tile_layer& tile_layer,
	augs::vector_wrapper<int>& shining,
	resources::renderable::draw_input& in) {

	auto visible_tiles = tile_layer.get_visible_tiles(in);
	tile_layer.generate_indices_by_type(visible_tiles);

	int random_type = shining.at(randval(int(0), int(shining.size() - 1)));
	if (int(tile_layer.indices_by_type.size()) - 1 < random_type) return vec2i(-1, -1);
	if (tile_layer.indices_by_type[random_type].empty()) return vec2i(-1, -1);
	
	return tile_layer.indices_by_type[random_type][randval(int(0), int(tile_layer.indices_by_type[random_type].size() - 1))];
}