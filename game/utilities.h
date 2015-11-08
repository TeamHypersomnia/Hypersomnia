#pragma once
#include "game_framework/resources/render_info.h"
#include "game_framework/resources/animate_info.h"

#include "misc/vector_wrapper.h"

extern void draw_tile_highlights(
	resources::tile_layer&,
	resources::animation&,
	std::vector<int>&,
	resources::renderable::draw_input&); 

extern vec2i get_random_coordinate_on_a_special_tile (
		resources::tile_layer&,
		augs::vector_wrapper<int>&,
		resources::renderable::draw_input& in);
