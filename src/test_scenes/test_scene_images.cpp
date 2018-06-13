#include "augs/filesystem/file.h"
#include "augs/misc/pool/pool.h"

#include "view/viewables/image_meta.h"
#include "view/viewables/image_definition.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_images.h"

#include "augs/templates/enum_introspect.h"
#include "augs/readwrite/lua_readwrite_errors.h"
#include "view/try_load_meta_lua.h"

void load_test_scene_images(
	sol::state& lua,
	image_definitions_map& all_definitions
) {
	using test_id_type = test_scene_image_id;

	all_definitions.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
		const auto id = to_image_id(enum_id);

		if (found_in(all_definitions, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(enum_id));

		image_definition definition;
		definition.set_source_path({ stem + ".png", true });

		try {
			try_load_meta_lua(
				lua, 
				definition.meta, 
				image_definition_view({}, definition).get_source_image_path()
			);
		}
		catch (const augs::lua_deserialization_error& err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nNot a valid lua table.\n%x",
				stem,
				err.what()
			);
		}
		catch (const augs::file_open_error& err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nFile might be corrupt.\n%x",
				stem,
				err.what()
			);
		}

		const auto new_allocation = all_definitions.allocate(std::move(definition));
		ensure_eq(id, new_allocation.key);
	});
}