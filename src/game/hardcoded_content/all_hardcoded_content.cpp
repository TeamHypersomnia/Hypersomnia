#include "generated/setting_build_test_scenes.h"
#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

#include "application/config_lua_table.h"

#include "application/content_generation/texture_atlases.h"
#include "application/content_generation/neon_maps.h"
#include "application/content_generation/desaturations.h"
#include "application/content_generation/buttons_with_corners.h"
#include "application/content_generation/scripted_images.h"

#include "augs/graphics/OpenGL_includes.h"
#include "augs/misc/lua_readwrite.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "generated/introspectors.h"

using namespace augs::graphics;
using namespace assets;

void regenerate_what_is_needed_for(const game_image_definitions& defs, const bool force_regenerate) {
	for (const auto& def : defs) {
		def.second.regenerate_resources(def.first, force_regenerate);
	}
}

void create_default_definitions_in_dir(const std::string& dir_path) {
	augs::for_each_file_in_dir_recursive(
		dir_path,
		[](const std::string& path) {
			if (augs::get_extension(path) == ".png") {
				const auto path_of_meta = augs::replace_filename(path, ".lua");

				if (!augs::file_exists(path_of_meta)) {
					augs::create_text_file(path_of_meta, "return {}");
				}
			}
		}
	);
}

template <class T>
void add_definitions_recursively_from_dir(const std::string& dir_path, T& into) {
	augs::for_each_file_in_dir_recursive(
		dir_path,
		[&into](const std::string& path) {
			if (augs::get_extension(path) == ".lua") {
				augs::load_from_lua_table(into[path], path);
			}
		}
	);
}

void load_all_requisite(const config_lua_table& cfg) {
	auto& manager = get_assets_manager();

	game_image_definitions images;
	game_font_definitions fonts;

	add_definitions_recursively_from_dir("content/requisite/gfx/", images);
	add_definitions_recursively_from_dir("content/requisite/fonts/", fonts);

#if BUILD_TEST_SCENES
	add_definitions_recursively_from_dir("content/official/gfx/", images);
	add_definitions_recursively_from_dir("content/official/fonts/", fonts);
#endif
	
	/*
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
	*/

	LOG("\n--------------------------------------------\nChecking content integrity...");

	const bool force_regenerate = cfg.debug_regenerate_content_every_launch;
	regenerate_what_is_needed_for(images, force_regenerate);

	atlases_regeneration_input in;

	for (const auto& i : images) {
		concatenate(in.images, i.second.get_atlas_inputs(i.first));
	}

	for (const auto& f : fonts) {
		in.fonts.push_back({ f.second.loading_input, f.second.target_atlas });
	}

	const auto atlases = regenerate_atlases(
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
		atlases
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
