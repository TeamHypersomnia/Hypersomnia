#pragma once
#include "augs/texture_atlas/texture_atlas.h"
#include "view/viewables/all_viewables_declarations.h"
#include "view/necessary_resources.h"

struct standard_atlas_distribution_input {
	const game_image_loadables_map& game_image_loadables;
	const necessary_image_definitions& necessary_image_loadables;
	const augs::font_loading_input& gui_font_input;
	const atlas_regeneration_settings settings;

	augs::graphics::texture& output_game_world_atlas;
	game_images_in_atlas_map& output_game_images;
	necessary_images_in_atlas& output_necessary_images;
	augs::baked_font& output_gui_font;
};

void standard_atlas_distribution(const standard_atlas_distribution_input in);