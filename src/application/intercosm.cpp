#include "intercosm.h"

#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/byte_file.h"

#if BUILD_TEST_SCENES
void intercosm::make_test_scene(sol::state& lua, const bool minimal) {
	world.clear();
	logicals = {};
	viewables = {};
	world.reserve_storage_for_entities(3000u);

	populate_test_scene_assets(lua, logicals, viewables);

	if (minimal) {
		test_scenes::minimal_scene().populate(world.common);
		locally_viewed = test_scenes::minimal_scene().populate_with_entities(make_logic_step_input({}));
	}
	else {
		test_scenes::testbed().populate(world.common);
		locally_viewed = test_scenes::testbed().populate_with_entities(make_logic_step_input({}));
	}
}
#endif

void intercosm::make_blank() {
	world.clear();
	world.reserve_storage_for_entities(100);

	auto origin = world.create_entity("origin_entity");
	origin += components::transform();

	locally_viewed = origin;
}

void intercosm::save(const intercosm_path_op op) const {
	auto target_extension = op.path.extension();

	if (target_extension == ".unsaved") {
		target_extension = augs::path_type(op.path).replace_extension("").extension();
	}

	if (target_extension == ".int") {
		augs::save_as_bytes(*this, op.path);
	}
	else if (target_extension == ".lua") {
		augs::save_as_lua_table(op.lua, *this, op.path);
	}
}

void intercosm::open(const intercosm_path_op op) {
	const auto display_path = augs::to_display_path(op.path);

	try {
		auto target_extension = op.path.extension();

		if (target_extension == ".unsaved") {
			/* Get the extension before ".unsaved" */
			target_extension = augs::path_type(op.path).replace_extension("").extension();
		}

		augs::recursive_clear(version);
		version.commit_number = 0;

		if (target_extension == ".int") {
			augs::load_from_bytes(*this, op.path);
		}
		else if (target_extension == ".lua") {
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