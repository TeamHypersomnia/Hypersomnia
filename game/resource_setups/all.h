#pragma once
#include "application/content_generation/texture_atlases.h"
#include "game/resources/manager.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/pixel.h"
#include "game/flyweights/spell_data.h"
#include "game/flyweights/physical_material.h"

class config_lua_table;

namespace resource_setups {
	void load_standard_everything(const config_lua_table&);

	game_image_requests load_standard_images();
	game_font_requests load_standard_fonts();

	void load_standard_particle_effects();
	void load_standard_behaviour_trees();
	void load_standard_tile_layers();
	void load_standard_sound_buffers();
}

void set_standard_collision_sound_matrix(collision_sound_matrix_type&);
void set_standard_spell_properties(augs::enum_associative_array<spell_type, spell_data>&);