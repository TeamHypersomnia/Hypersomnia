#pragma once
#include "augs/texture_atlas/texture_atlas.h"
#include "augs/templates/exception_templates.h"

#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;
struct all_viewables;
struct cosmos_common_state;

// test scene content

struct test_scene_asset_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

void load_test_scene_sound_buffers(sound_buffer_inputs_map&);
void load_test_scene_particle_effects(
	const game_image_loadables_map&,
	particle_effects_map&
);
void load_test_scene_images(
	game_image_loadables_map&,
	game_image_metas_map&
);

void load_test_scene_animations(all_logical_assets&);
void load_test_scene_physical_materials(all_logical_assets&);
void load_test_scene_recoil_players(all_logical_assets&);

void populate_test_scene_assets(
	all_logical_assets& output_logicals,
	all_viewables& output_sources
);

void load_test_scene_sentience_properties(cosmos_common_state&);
