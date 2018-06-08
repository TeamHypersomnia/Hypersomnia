#pragma once
#include "augs/texture_atlas/bake_fresh_atlas.h"
#include "augs/templates/exception_templates.h"

#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_declarations.h"

struct all_logical_assets;
struct all_viewables_defs;
struct cosmos_common_significant;

// test scene content

struct test_scene_asset_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

class loaded_image_caches_map;

void load_test_scene_sounds(sound_definitions_map&);
void load_test_scene_particle_effects(
	const loaded_image_caches_map&,
	const plain_animations_pool& anims,
	particle_effects_map&
);

namespace sol {
	class state;
}

void load_test_scene_images(
	sol::state& lua,
	image_definitions_map&
);

void load_test_scene_animations(all_logical_assets&);
void load_test_scene_physical_materials(physical_materials_pool&);
void load_test_scene_recoil_players(recoil_players_pool&);

void load_test_scene_sentience_properties(cosmos_common_significant&);

/* Top-level populators */

loaded_image_caches_map populate_test_scene_images_and_sounds(
	sol::state& lua,
	all_viewables_defs& output_sources
); 

void populate_test_scene_logical_assets(
	all_logical_assets& output_logicals
); 

void populate_test_scene_viewables(
	const loaded_image_caches_map& caches,
	const plain_animations_pool& anims,
	all_viewables_defs& output_sources
);

void populate_test_scene_common(const loaded_image_caches_map&, cosmos_common_significant&);
