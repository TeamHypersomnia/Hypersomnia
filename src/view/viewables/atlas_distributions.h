#pragma once
#include "augs/texture_atlas/atlas_generation.h"

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

struct atlas_profiler;

struct standard_atlas_distribution_input {
	const image_definitions_map& image_definitions;
	const necessary_image_definitions_map& necessary_image_definitions;
	const augs::font_loading_input& gui_font_input;
	const atlas_regeneration_settings settings;
	const augs::path_type unofficial_project_dir;

	images_in_atlas_map& output_atlas_entries;
	necessary_images_in_atlas_map& output_necessary_atlas_entries;
	augs::baked_font& output_gui_font;

	atlas_profiler& profiler;
	augs::time_measurements& atlas_upload_to_gpu;
};

struct standard_atlas_distribution {

};

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in);