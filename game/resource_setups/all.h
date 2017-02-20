#pragma once
#include "game/resources/manager.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/pixel.h"

namespace resource_setups {
	void load_standard_everything();

	void load_standard_atlas();
	void load_standard_particle_effects();
	void load_standard_behaviour_trees();
	void load_standard_tile_layers();
	void load_standard_sound_buffers();
	
	void make_button_with_corners(
		resources::manager& manager, 
		const assets::texture_id inside_tex,

		const rgba border_color,
		const rgba inside_color,

		const int lower_side,
		const int upper_side,

		const int inside_border_padding,
		const bool make_lb_complement
	);
}