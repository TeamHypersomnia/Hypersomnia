#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"
#include "test_scenes/test_scenes_content.h"
#include "view/viewables/game_image.h"

#if BUILD_TEST_SCENES
loaded_game_image_caches_map populate_test_scene_images_and_sounds(
	sol::state& lua,
	all_viewables_defs& output_sources
) {
	auto& loadables = output_sources.image_loadables;
	auto& metas = output_sources.image_metas;

	try {
		load_test_scene_images(lua, loadables, metas);
		load_test_scene_sound_buffers(output_sources.sounds);
	}
	catch (const test_scene_asset_loading_error err) {
		LOG(err.what());
	}

	return loaded_game_image_caches_map(loadables, metas);
}

void populate_test_scene_logical_assets(
	all_logical_assets& output_logicals
) {
	load_test_scene_animations(output_logicals);
	load_test_scene_physical_materials(output_logicals);
	load_test_scene_recoil_players(output_logicals);
}

void populate_test_scene_viewables(
	sol::state& lua,
	const loaded_game_image_caches_map& caches,
	all_viewables_defs& output_sources
) {
	try {
		load_test_scene_particle_effects(
			caches,
			output_sources.particle_effects
		);
	}
	catch (const test_scene_asset_loading_error err) {
		LOG(err.what());
	}
}
#endif
