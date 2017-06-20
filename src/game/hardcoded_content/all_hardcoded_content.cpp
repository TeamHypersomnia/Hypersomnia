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
#include "augs/misc/lua_readwrite.h"

#include "generated/introspectors.h"

using namespace augs::graphics;
using namespace assets;

game_image_definitions load_game_image_definitions(const std::string& lua_file_path) {

}

void load_all_requisite(const config_lua_table& cfg) {
	auto& manager = get_assets_manager();

	game_image_definitions images;
	const auto fonts = augs::load_from_lua_table<game_font_requests>("official/requisite_font_definitions.lua");

	augs::for_each_enum<game_image_id>(
		[&images](const game_image_id id){
			if (
				id != game_image_id::INVALID
				&& id != game_image_id::REQUISITE_COUNT
				&& id != game_image_id::BLANK
				&& id != game_image_id::COUNT
			) {
				const std::string stem = augs::enum_to_string(id);
				game_image_definition rq;
				rq.source_image_path = stem;
				images[stem] = rq;
			}
		}
	);

	augs::save_as_lua_table(images, "imgs.lua");

#if BUILD_TEST_SCENES
	const auto test_scene_images = load_test_scene_images();
	concatenate(images, test_scene_images);
#endif

	atlases_regeneration_input in;

	for (const auto& i : images) {
		const auto& request = i.second;

		in.images.push_back({ request.source_image_path, assets::gl_texture_id::GAME_WORLD_ATLAS });

		if (request.neon_map) {
			in.images.push_back({ request.get_neon_map_path(), assets::gl_texture_id::GAME_WORLD_ATLAS });
		}

		if (request.generate_desaturation) {
			in.images.push_back({ request.get_desaturation_path(), assets::gl_texture_id::GAME_WORLD_ATLAS });
		}
	}

	for (const auto& f : fonts) {
		in.fonts.push_back({ f.second.loading_input, f.second.target_atlas });
	}

	LOG("\n--------------------------------------------\nChecking content integrity...");


	//const std::string neon_directory = "generated/neon_maps/";
	//augs::create_directories(neon_directory);

	regenerate_neon_map("", "", {}, false); //cfg.debug_regenerate_content_every_launch);
	regenerate_scripted_images(cfg.debug_regenerate_content_every_launch);
	regenerate_buttons_with_corners(cfg.debug_regenerate_content_every_launch);
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
