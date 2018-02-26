#include "intercosm.h"

#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"
#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/byte_file.h"

#if BUILD_TEST_SCENES
void intercosm::make_test_scene(
	sol::state& lua, 
	const test_scene_settings settings
) {
	world.clear();
	viewables = {};

#if !STATICALLY_ALLOCATE_ENTITIES
	cosmic::reserve_storage_for_entities(world, 3000u);
#endif

	const auto caches = populate_test_scene_images_and_sounds(lua, viewables);
	populate_test_scene_viewables(lua, caches, viewables);

	auto reloader = [&](auto populator){
		world.change_common_significant([&](cosmos_common_significant& common){
			auto& logicals = common.logical_assets;
			logicals = {};
			populate_test_scene_logical_assets(logicals);

			populator.populate(caches, common);

			return changer_callback_result::DONT_REFRESH;
		});

		cosmic::change_solvable_significant(world, [settings](auto& s){
			s.clock.delta = augs::delta::steps_per_second(settings.scene_tickrate); 
			return changer_callback_result::REFRESH;
		});

		locally_viewed = populator.populate_with_entities(caches, make_logic_step_input({}));
	};

	if (settings.create_minimal) {
		reloader(test_scenes::minimal_scene());
	}
	else {
		reloader(test_scenes::testbed());
	}
}
#endif

static auto get_effective_extension(const augs::path_type& file_path) {
	const auto file_extension = file_path.extension();

	if (file_extension == ".unsaved") {
		/* Eat the .unsaved suffix to get the effective extension */
		return augs::path_type(file_path).replace_extension("").extension();
	}

	return file_extension;
}

void intercosm::save(const intercosm_path_op op) const {
	const auto file_extension = op.path.extension();
	const auto effective_extension = get_effective_extension(op.path);

	if (effective_extension == ".int") {
		augs::save_as_bytes(*this, op.path);
	}
	else if (effective_extension == ".lua") {
		augs::save_as_lua_table(op.lua, *this, op.path);
	}

	if (const auto unneeded_backup_path = augs::path_type(op.path) += ".unsaved";
		file_extension != ".unsaved" && augs::file_exists(unneeded_backup_path)
	) {
		/* Clear the backup file */
		augs::remove_file(unneeded_backup_path);
	}
}

void intercosm::open(const intercosm_path_op op) {
	const auto display_path = augs::to_display_path(op.path);

	try {
		const auto file_extension = op.path.extension();
		const auto effective_extension = get_effective_extension(op.path);

		augs::recursive_clear(version);
		version.commit_number = 0;

		if (effective_extension == ".int") {
			augs::load_from_bytes(*this, op.path);
		}
		else if (effective_extension == ".lua") {
			augs::load_from_lua_table(op.lua, *this, op.path);
		}

		/* TODO: Check version integrity */

		version = hypersomnia_version();
	}
	catch (const cosmos_loading_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::stream_read_error err) {
		throw intercosm_loading_error{
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::lua_deserialization_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nNot a valid lua table.", display_path),
			err.what()
		};
	}
	catch (const augs::ifstream_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be missing.", display_path),
			err.what()
		};
	}
}