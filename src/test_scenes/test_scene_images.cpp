#include "augs/filesystem/file.h"
#include "augs/misc/pool/pool.h"

#include "view/viewables/image_structs.h"
#include "view/viewables/regeneration/image_loadables_def.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_images.h"

#include "augs/readwrite/lua_file.h"
#include "augs/templates/enum_introspect.h"

void load_test_scene_images(
	sol::state& lua,
	image_loadables_map& all_loadables,
	image_metas_map& all_metas
) {
	using id_type = assets::image_id;

	const auto directory = augs::path_type("content/official/gfx/");

	augs::for_each_enum_except_bounds([&](const test_scene_image_id enum_id) {
		const auto id = to_image_id(enum_id);

		if (found_in(all_loadables, id) || found_in(all_metas, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(enum_id));

		image_loadables_def loadables_def;
		image_meta meta;

		loadables_def.source_image_path = augs::path_type(directory) += stem + ".png";

		try {
			if (const auto extra_loadables_path = augs::path_type(directory) += stem + ".extras.lua";
				augs::exists(extra_loadables_path)
			) {
				augs::load_from_lua_table(lua, loadables_def.extras, extra_loadables_path);
			}

			if (const auto meta_path = augs::path_type(directory) += stem + ".meta.lua";
				augs::exists(meta_path)
			) {
				augs::load_from_lua_table(lua, meta, meta_path);
			}
		}
		catch (augs::lua_deserialization_error err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nNot a valid lua table.\n%x",
				stem,
				err.what()
			);
		}
		catch (augs::file_open_error err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nFile might be corrupt.\n%x",
				stem,
				err.what()
			);
		}

		{
			const auto new_allocation = all_loadables.allocate(std::move(loadables_def));
			ensure(new_allocation.key == id);
		}

		{
			const auto new_allocation = all_metas.allocate(std::move(meta));
			ensure(new_allocation.key == id);
		}
	});
}