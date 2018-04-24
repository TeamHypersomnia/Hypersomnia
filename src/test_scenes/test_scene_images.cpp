#include "augs/filesystem/file.h"
#include "augs/misc/pool/pool.h"

#include "view/viewables/image_in_atlas.h"
#include "view/viewables/image_meta.h"
#include "view/viewables/regeneration/image_definition.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_images.h"

#include "augs/readwrite/lua_file.h"
#include "augs/templates/enum_introspect.h"

static void try_load_lua_neighbors(
	const augs::path_type& resolved,
   	sol::state& lua,
   	image_extra_loadables& extras,
	image_meta& meta
) {
	const auto with_ext = [&](const auto ext) {
		return augs::path_type(resolved).replace_extension(ext);
	};

	if (const auto extra_loadables_path = with_ext(".extras.lua");
		augs::exists(extra_loadables_path)
	) {
		augs::load_from_lua_table(lua, extras, extra_loadables_path);
	}

	if (const auto meta_path = with_ext(".meta.lua");
		augs::exists(meta_path)
	) {
		augs::load_from_lua_table(lua, meta, meta_path);
	}
}

void load_test_scene_images(
	sol::state& lua,
	image_definitions_map& all_definitions
) {
	using id_type = assets::image_id;

	augs::for_each_enum_except_bounds([&](const test_scene_image_id enum_id) {
		const auto id = to_image_id(enum_id);

		if (found_in(all_definitions, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(enum_id));

		image_definition definition;
		auto& loadables_def = definition.loadables;
		image_meta meta;

		definition.set_source_path({ stem + ".png", true });

		try {
			try_load_lua_neighbors(image_definition_view({}, definition).get_source_image_path(), lua, loadables_def.extras, meta);
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

		const auto new_allocation = all_definitions.allocate(std::move(definition));
		ensure_eq(new_allocation.key, id);
	});
}