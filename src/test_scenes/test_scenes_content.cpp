#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"
#include "test_scenes/test_scenes_content.h"
#include "view/viewables/image_structs.h"

#if BUILD_TEST_SCENES
loaded_image_caches_map populate_test_scene_images_and_sounds(
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

	loaded_image_caches_map out;

	loadables.for_each_object_and_id(
		[&](const auto& object, const auto id) {
			out.try_emplace(id, image_loadables_def_view({}, object), metas.at(id));
		}
	);

	return out;
}

void populate_test_scene_logical_assets(
	all_logical_assets& output_logicals
) {
	load_test_scene_animations(output_logicals.animations);
	load_test_scene_physical_materials(output_logicals.physical_materials);
	load_test_scene_recoil_players(output_logicals.recoils);
}

void populate_test_scene_viewables(
	sol::state& lua,
	const loaded_image_caches_map& caches,
	const animations_pool& anims,
	all_viewables_defs& output_sources
) {
	try {
		load_test_scene_particle_effects(
			caches,
			anims,
			output_sources.particle_effects
		);
	}
	catch (const test_scene_asset_loading_error err) {
		LOG(err.what());
	}
}
#endif
