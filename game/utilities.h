#pragma once
#include "game_framework/shared/drawing_state.h"
#include "game_framework/resources/animate_info.h"
#include "game_framework/components/tile_layer_component.h"

#include "misc/vector_wrapper.h"

extern void draw_tile_highlights(
	components::tile_layer&,
	resources::animation&,
	std::vector<int>&,
	shared::drawing_state&); 

extern vec2i get_random_coordinate_on_a_special_tile (
	components::tile_layer&,
		augs::vector_wrapper<int>&,
		shared::drawing_state& in);
