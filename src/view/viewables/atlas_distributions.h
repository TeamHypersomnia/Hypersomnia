#pragma once
#include "augs/texture_atlas/texture_atlas.h"

#include "view/necessary_resources.h"
#include "view/viewables/all_viewables_declarations.h"

/* 
	The standard atlas distribution is the simplest one where
	all necessary images, game images and gui font reside in a single atlas called
	"game_world_atlas".

	As the game develops, different rendering scripts might need more atlases,
	for example a separate one for all item sprites. We will add 
	such atlas distributions as needed, to this file.
*/

struct standard_atlas_distribution_input {
	const game_image_loadables_map& image_loadables;
	const necessary_image_loadables_map& necessary_image_loadables;
	const augs::font_loading_input& gui_font_input;
	const atlas_regeneration_settings settings;

	game_images_in_atlas_map& output_game_images;
	necessary_images_in_atlas_map& output_necessary_images;
	augs::baked_font& output_gui_font;
};

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in);