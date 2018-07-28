#pragma once
#include "augs/texture_atlas/bake_fresh_atlas.h"

#include "view/necessary_resources.h"
#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/images_in_atlas_map.h"
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

	/* Necessary definitions are initialized once, and never modified later on. */
	const necessary_image_definitions_map& necessary_image_definitions;

	const image_definitions_map image_definitions;
	const augs::font_loading_input& gui_font_input;
	const augs::path_type unofficial_project_dir;
};

struct general_atlas_input {
	subjects_gathering_input subjects;
	const unsigned max_atlas_size;

	rgba* const atlas_image_output;
	std::vector<rgba>& fallback_output;
};

struct general_atlas_output {
	images_in_atlas_map atlas_entries;
	necessary_images_in_atlas_map necessary_atlas_entries;
	augs::baked_font gui_font;
	vec2u atlas_size;
};

struct atlas_input_subjects;

void regenerate_and_gather_subjects(
	subjects_gathering_input,
	atlas_input_subjects& output,
	augs::time_measurements& neon_regeneration_performance
);

general_atlas_output create_general_atlas(
	general_atlas_input in,
	atlas_profiler&,
	augs::time_measurements& neon_regeneration_performance
);