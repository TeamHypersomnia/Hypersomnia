#pragma once
#include "augs/texture_atlas/texture_atlas.h"
#include "augs/templates/exception_templates.h"

#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;
struct all_viewables_defs;
struct cosmos_common_significant;

// test scene content

struct test_scene_asset_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

struct loaded_game_image_caches_map;

void load_test_scene_sound_buffers(sound_buffer_inputs_map&);
void load_test_scene_particle_effects(
	const loaded_game_image_caches_map&,
	particle_effects_map&
);

namespace sol {
	class state;
}

void load_test_scene_images(
	sol::state& lua,
	image_loadables_map&,
	image_metas_map&
);

void load_test_scene_animations(all_logical_assets&);
void load_test_scene_physical_materials(all_logical_assets&);
void load_test_scene_recoil_players(all_logical_assets&);

void load_test_scene_sentience_properties(cosmos_common_significant&);

/* Top-level populators */

loaded_game_image_caches_map populate_test_scene_images_and_sounds(
	sol::state& lua,
	all_viewables_defs& output_sources
); 

void populate_test_scene_logical_assets(
	all_logical_assets& output_logicals
); 

void populate_test_scene_viewables(
	sol::state& lua,
	const loaded_game_image_caches_map& caches,
	all_viewables_defs& output_sources
);

