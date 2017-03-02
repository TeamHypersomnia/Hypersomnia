#pragma once
#include "game/resources/requested_atlas_resources.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/pixel.h"
#include "game/flyweights/spell_data.h"
#include "game/flyweights/physical_material.h"


namespace resource_setups {
	void load_standard_everything();

	std::unordered_map<assets::texture_id, all_information_about_image> load_standard_images();
	std::unordered_map<assets::font_id, all_information_about_font> load_standard_fonts();

	void load_standard_image_settings();

	void load_standard_particle_effects();
	void load_standard_behaviour_trees();
	void load_standard_tile_layers();
	void load_standard_sound_buffers();
}

void set_standard_collision_sound_matrix(collision_sound_matrix_type&);
void set_standard_spell_properties(augs::enum_associative_array<spell_type, spell_data>&);