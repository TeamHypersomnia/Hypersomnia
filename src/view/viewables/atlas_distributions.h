#pragma once
#include "augs/texture_atlas/bake_fresh_atlas.h"

#include "view/necessary_resources.h"
#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"

/* 
	The standard atlas distribution is the simplest one where
	all necessary images, game images and gui font reside in a single atlas called
	"game_world_atlas".

	As the game develops, different rendering scripts might need more atlases,
	for example a separate one for all item sprites. We will add 
	such atlas distributions as needed, to this file.
*/

struct atlas_profiler;

struct subjects_gathering_input {
	const content_regeneration_settings settings;
	const necessary_image_definitions_map& necessary_image_definitions;
	const image_definitions_map& image_definitions;
	const augs::font_loading_input& gui_font_input;
	const augs::path_type unofficial_project_dir;
};

struct standard_atlas_distribution_input {
	subjects_gathering_input subjects;
	const unsigned max_atlas_size;
};

struct standard_atlas_distribution_output {
	images_in_atlas_map& atlas_entries;
	necessary_images_in_atlas_map& necessary_atlas_entries;
	augs::baked_font& gui_font;

	atlas_profiler& profiler;
	augs::time_measurements& atlas_upload_to_gpu;
};

struct atlas_input_subjects;

void regenerate_and_gather_subjects(
	subjects_gathering_input,
	atlas_input_subjects& output
);

struct standard_atlas_distribution {
	augs::graphics::texture general;
	// augs::graphics::texture neon_maps;
};

standard_atlas_distribution create_standard_atlas_distribution(
	standard_atlas_distribution_input in,
	standard_atlas_distribution_output out
);