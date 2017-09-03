#pragma once
#include <tuple>
#include "augs/texture_atlas/texture_atlas.h"
#include "augs/templates/exception_templates.h"

struct content_regeneration_settings;
class all_logical_assets;
class all_viewable_defs;
struct cosmos_common_state;

// test scene content

struct test_scene_asset_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

void load_test_scene_sound_buffers(all_viewable_defs&);
void load_test_scene_particle_effects(all_viewable_defs&);
void load_test_scene_images(all_viewable_defs&);

void load_test_scene_animations(all_logical_assets&);
void load_test_scene_physical_materials(all_logical_assets&);
void load_test_scene_recoil_players(all_logical_assets&);

void populate_test_scene_assets(
	all_logical_assets& output_logicals,
	all_viewable_defs& output_sources
);

void load_test_scene_sentience_properties(cosmos_common_state&);
