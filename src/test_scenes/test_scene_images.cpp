#include "augs/ensure_rel_util.h"
#include "augs/filesystem/file.h"
#include "augs/misc/pool/pool.h"
#include "augs/misc/pool/pool_allocate.h"

#include "view/viewables/image_meta.h"
#include "view/viewables/image_definition.h"
#include "view/viewables/all_viewables_defs.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_images.h"

#include "test_scenes/test_enum_to_path.h"
#include "augs/readwrite/lua_readwrite_errors.h"
#include "view/load_meta_lua.h"

#include "augs/log_direct.h"
#include "augs/filesystem/find_path.h"

struct test_image_does_not_exist {

};

void load_test_scene_images(
	sol::state& lua,
	image_definitions_map& all_definitions
) {
	using test_id_type = test_scene_image_id;

	all_definitions.reserve(enum_count(test_id_type()));

	auto setup_with_path = [&all_definitions, &lua](const augs::path_type& final_path) {
		image_definition new_def;
		new_def.set_source_path({ final_path.string(), true });

		try {
			load_meta_lua_if_exists(
				lua, 
				new_def.meta, 
				image_definition_view({}, new_def).get_source_image_path()
			);
		}
		catch (const augs::lua_deserialization_error& err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nNot a valid lua table.\n%x",
				final_path,
				err.what()
			);
		}

		return all_definitions.allocate(std::move(new_def));
	};

	auto register_image = [&](augs::path_type at_stem) -> decltype(auto) {
		at_stem = augs::path_type("gfx") / at_stem;

		image_definition definition;
		definition.set_source_path({ at_stem.string(), true });

		const auto resolved_no_ext = definition.get_source_path().resolve({});
		const auto with_existent_ext = augs::first_existing_extension(resolved_no_ext, augs::IMAGE_EXTENSIONS);

		if (with_existent_ext.empty()) {
			throw test_image_does_not_exist{};
		}

		auto final_path = at_stem;
		final_path += with_existent_ext.extension();

		return setup_with_path(final_path);
	};

	augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
		const auto id = to_image_id(enum_id);

		if (found_in(all_definitions, id)) {
			return;
		}

		auto stem = enum_to_image_stem(enum_id);

		try {
			const auto new_allocation = register_image(stem);
			(void)new_allocation;
			(void)id;

			ensure_eq_id(id, new_allocation.key);
		}
		catch (const test_image_does_not_exist&) {
			LOG_NOFORMAT("WARNING! Could not find an official game image: " + stem);
		}
	});

	/* Load subsequent frames if they exist. */

	const auto& to_enum = augs::get_string_to_enum_map<test_id_type>();

	augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
		{
			const auto stringized_enum = augs::enum_to_string(enum_id);

			if (mapped_or_nullptr(to_enum, cut_trailing_number(stringized_enum) + "2") != nullptr) {
				/* Don't do this for enums that specify frame numbers directly. */
				return;
			}
		}

		auto stem = enum_to_image_stem(enum_id);

		if (::get_trailing_number(stem) == 1u) {
			cut_trailing_number(stem);

			for (int i = 2; ; ++i) {
				const auto next_stem = stem + std::to_string(i);

				try {
					register_image(next_stem);
				}
				catch (const test_image_does_not_exist&) {
					break;
				}
			}
		}
	});
}