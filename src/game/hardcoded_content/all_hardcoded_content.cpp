#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

#include "application/content_regeneration/content_regeneration_settings.h"

#include "application/content_regeneration/texture_atlases.h"
#include "application/content_regeneration/neon_maps.h"
#include "application/content_regeneration/desaturations.h"
#include "application/content_regeneration/buttons_with_corners.h"
#include "application/content_regeneration/scripted_images.h"

#include "augs/misc/lua_readwrite.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/gui/button_corner_type.h"
#include "augs/gui/button_corners.h"
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
				const auto path_of_meta = augs::replace_extension(path, ".lua");

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

void load_all_requisite(assets_manager& manager, const content_regeneration_settings& cfg) {
	game_image_definitions images;
	game_font_definitions fonts;

	add_definitions_recursively_from_dir("content/requisite/gfx/", images);
	add_definitions_recursively_from_dir("content/requisite/fonts/", fonts);

#if BUILD_TEST_SCENES
	add_definitions_recursively_from_dir("content/official/gfx/", images);
	add_definitions_recursively_from_dir("content/official/fonts/", fonts);
#endif

	for (auto& f : fonts) {
		// let the font definitions in .lua specify "path" relatively to the font definition file path
		// usually just font name
		const auto full_definition_path = f.first;

		{
			auto& relative_path = f.second.loading_input.path;
			relative_path = augs::replace_filename(full_definition_path, relative_path);
		}
		{
			auto& relative_path = f.second.loading_input.input.charset_path;
			relative_path = augs::replace_filename(full_definition_path, relative_path);
		}
	}

	LOG("\n--------------------------------------------\nChecking content integrity...");

	const bool force_regenerate = cfg.regenerate_every_launch;
	
	load_button_with_corners(
		cfg.menu_button,
		images,
		assets::game_image_id::MENU_BUTTON_INSIDE,
		"generated/content/requisite/gfx/buttons_with_corners/menu_button_%x.png",
		force_regenerate
	);

	load_button_with_corners(
		cfg.hotbar_button,
		images,
		assets::game_image_id::HOTBAR_BUTTON_INSIDE,
		"generated/content/requisite/gfx/buttons_with_corners/hotbar_button_%x.png",
		force_regenerate
	);

	regenerate_what_is_needed_for(images, force_regenerate);

	atlases_regeneration_input in;

	for (const auto& i : images) {
		concatenate(in.images, i.second.get_atlas_inputs(i.first));
	}

	for (const auto f : fonts) {
		in.fonts.push_back({ f.second.loading_input, f.second.target_atlas });
	}

	const auto atlases = regenerate_atlases(
		in,
		force_regenerate,
		cfg.check_integrity_every_launch,
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
	load_test_scene_recoil_players(manager);
#endif
}
