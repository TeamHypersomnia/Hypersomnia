#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"
#include "test_scenes/test_scenes_content.h"

void populate_test_scene_assets(
	sol::state& lua,
	all_logical_assets& output_logicals,
	all_viewables_defs& output_sources
) {
#if BUILD_TEST_SCENES
	try {
		load_test_scene_images(
			lua,
			output_sources.game_image_loadables,
			output_sources.game_image_metas
		);

		load_test_scene_particle_effects(
			output_sources.game_image_loadables, 
			output_sources.particle_effects
		);

		load_test_scene_sound_buffers(output_sources.sounds);

		load_test_scene_animations(output_logicals);
		load_test_scene_physical_materials(output_logicals);
		load_test_scene_recoil_players(output_logicals);
	}
	catch (const test_scene_asset_loading_error err) {
		LOG(err.what());
		press_any_key_and_exit();
	}
#endif

	output_sources.update_into(output_logicals);
}