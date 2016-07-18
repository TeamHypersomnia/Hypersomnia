#pragma once
#include "game/detail/state_for_drawing_camera.h"
#include "game/resources/animation.h"
#include "game/components/tile_layer_component.h"

#include "misc/vector_wrapper.h"

extern void draw_tile_highlights(
	components::tile_layer&,
	resources::animation&,
	std::vector<int>&,
	shared::state_for_drawing_renderable&); 

extern vec2i get_random_coordinate_on_a_special_tile (
	components::tile_layer&,
		augs::vector_wrapper<int>&,
		shared::state_for_drawing_renderable& in);
