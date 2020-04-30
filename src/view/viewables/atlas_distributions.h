#pragma once
#include "augs/texture_atlas/bake_fresh_atlas.h"

#include "view/gui_fonts.h"
#include "view/necessary_resources.h"
#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "view/viewables/avatars_in_atlas_map.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "augs/texture_atlas/loaded_images_vector.h"

/* 
	The standard atlas distribution is the simplest one where
	all necessary images, game images and gui font reside in a single atlas called
	"game_world_atlas".

	As the game develops, different rendering scripts might need more atlases,
	for example a separate one for all item sprites. We will add 
	such atlas distributions as needed, to this file.
*/

struct atlas_profiler;
struct atlas_progress_structs;

struct subjects_gathering_input {
	const content_regeneration_settings settings;

	/* Necessary definitions are initialized once, and never modified later on. */
	const necessary_image_definitions_map& necessary_image_definitions;

	const image_definitions_map image_definitions;
	const all_gui_fonts_inputs gui_font_inputs;
	const augs::path_type unofficial_project_dir;
	atlas_progress_structs* const progress;
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
	all_loaded_gui_fonts gui_fonts;
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
	atlas_profiler&
);

struct avatar_atlas_output {
	avatars_in_atlas_map atlas_entries;
	vec2u atlas_size;
};

struct avatar_atlas_input {
	arena_player_metas subjects;
	const unsigned max_atlas_size;

	rgba* const atlas_image_output;
	std::vector<rgba>& fallback_output;
};

avatar_atlas_output create_avatar_atlas(avatar_atlas_input in);

struct ad_hoc_atlas_output {
	ad_hoc_in_atlas_map atlas_entries;
	vec2u atlas_size;
};

struct ad_hoc_atlas_input {
	ad_hoc_atlas_subjects subjects;
	const unsigned max_atlas_size;

	rgba* const atlas_image_output;
	std::vector<rgba>& fallback_output;
};

ad_hoc_atlas_output create_ad_hoc_atlas(ad_hoc_atlas_input in);
