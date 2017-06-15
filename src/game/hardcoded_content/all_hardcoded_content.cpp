#include "generated/setting_build_test_scenes.h"
#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

#include "application/config_lua_table.h"

#include "application/content_generation/texture_atlases.h"
#include "application/content_generation/neon_maps.h"
#include "application/content_generation/desaturations.h"
#include "application/content_generation/buttons_with_corners.h"
#include "application/content_generation/scripted_images.h"
#include "application/content_generation/polygonizations_of_images.h"

#include "augs/graphics/OpenGL_includes.h"

using namespace augs::graphics;
using namespace assets;

void load_all_requisite(const config_lua_table& cfg) {
	auto& manager = get_assets_manager();

	auto images = load_requisite_images();
	auto fonts = load_requisite_fonts();

#if BUILD_TEST_SCENES
	const auto test_scene_images = load_test_scene_images();
	concatenate(images, test_scene_images);
#endif

	atlases_regeneration_input in;

	for (const auto& i : images) {
		for (const auto& t : i.second.texture_maps) {
			if (t.path.size() > 0) {
				ensure(t.target_atlas != gl_texture_id::INVALID);

				in.images.push_back({ t.path, t.target_atlas });
			}
		}
	}

	for (const auto& f : fonts) {
		in.fonts.push_back({ f.second.loading_input, f.second.target_atlas });
	}

	LOG("\n--------------------------------------------\nChecking content integrity...");

	regenerate_scripted_images(cfg.debug_regenerate_content_every_launch);
	regenerate_buttons_with_corners(cfg.debug_regenerate_content_every_launch);
	regenerate_neon_maps(cfg.debug_regenerate_content_every_launch);
	regenerate_desaturations(cfg.debug_regenerate_content_every_launch);
	regenerate_polygonizations_of_images(cfg.debug_regenerate_content_every_launch);

	const auto regenerated = regenerate_atlases(
		in,
		cfg.debug_regenerate_content_every_launch,
		cfg.check_content_integrity_every_launch,
		cfg.save_regenerated_atlases_as_binary,
		cfg.packer_detail_max_atlas_size
	);

	LOG("Content regenerated successfully.\n--------------------------------------------\n");

	manager.load_baked_metadata(
		images,
		fonts,
		regenerated
	);

	load_requisite_animations(manager);
	load_requisite_sound_buffers(manager);

#if BUILD_TEST_SCENES
	load_test_scene_animations(manager);
	load_test_scene_sound_buffers(manager);
	load_test_scene_particle_effects(manager);
	load_test_scene_physical_materials(manager);
	load_test_scene_tile_layers(manager);
#endif
}
