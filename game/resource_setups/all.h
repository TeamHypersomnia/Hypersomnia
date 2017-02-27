#pragma once
#include "game/resources/manager.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/pixel.h"
#include "game/flyweights/spell_data.h"
#include "game/flyweights/physical_material.h"

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

		const unsigned lower_side,
		const unsigned upper_side,

		const unsigned inside_border_padding,
		const bool make_lb_complement
	);
}

void set_standard_collision_sound_matrix(collision_sound_matrix_type&);
void set_standard_spell_properties(augs::enum_associative_array<spell_type, spell_data>&);