#include "game/hardcoded_content/test_scenes/test_scenes_content.h"
#include "game/assets/all_assets.h"

void populate_test_scene_assets(
	all_logical_assets& output_logicals,
	all_viewable_defs& output_sources
) {
#if BUILD_TEST_SCENES
	try {
		load_test_scene_images(output_sources);
		load_test_scene_particle_effects(output_sources);
		load_test_scene_sound_buffers(output_sources);

		load_test_scene_animations(output_logicals);
		load_test_scene_physical_materials(output_logicals);
		load_test_scene_recoil_players(output_logicals);
	}
	catch (const test_scene_asset_loading_error err) {
		LOG(err.what());
		press_any_key_and_exit();
	}
#endif

	output_logicals.update_from(output_sources);
}