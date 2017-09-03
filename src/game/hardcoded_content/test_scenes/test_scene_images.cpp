#include "game/assets/all_assets.h"
#include "game/hardcoded_content/test_scenes/test_scenes_content.h"

#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/misc/lua_readwrite.h"

#include "generated/introspectors.h"

void load_test_scene_images(all_viewable_defs& into) {
	using id_type = assets::game_image_id;

	/* 
		This additional reference is only to mitigate MSVC bug 
		whereby there is some(?) problem capturing "this" contents in lambdas.
	*/

	auto& images = into.get_store_by(id_type());
	const auto directory = augs::path_type("content/official/gfx/");

	augs::for_each_enum_except_bounds<id_type>([&](const id_type id) {
		if (found_in(images, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(id));

		game_image_definition definition_template;

		if (
			const auto additional_properties_path = augs::path_type(directory) += stem + ".lua";
			augs::file_exists(additional_properties_path)
		) {
			try {
				augs::load_from_lua_table(
					definition_template,
					additional_properties_path
				);
			}
			catch (augs::lua_deserialization_error err) {
				throw test_scene_asset_loading_error(
					"Error while loading additional properties for %x (%x):",
					stem,
					additional_properties_path,
					err.what()
				);
			}
		}

		if (
			const auto source_image_path = augs::path_type(directory) += stem + ".png";
			augs::file_exists(source_image_path)
		) {
			definition_template.source_image_path = source_image_path;
			images.emplace(id, std::move(definition_template));
		}
	});
}