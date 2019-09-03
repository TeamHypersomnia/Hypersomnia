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

inline auto find_with_existent_extension(const augs::path_type& path_wo_extension) {
	const std::array<const char*, 5> exts = {
		".png", ".jpg", ".jpeg", ".tga", ".bmp"
	};

	for (const auto e : exts) {
		auto candidate = path_wo_extension;
		candidate += e;

		if (augs::exists(candidate)) {
			return candidate;
		}
	}

	return augs::path_type();
}

struct test_image_does_not_exist {

};

void load_test_scene_images(
	sol::state& lua,
	image_definitions_map& all_definitions
) {
	using test_id_type = test_scene_image_id;

	all_definitions.reserve(enum_count(test_id_type()));

	auto register_image = [&](const augs::path_type& at_stem) -> decltype(auto) {
		image_definition definition;
		definition.set_source_path({ at_stem.string(), true });

		const auto resolved_no_ext = definition.get_source_path().resolve({});
		const auto with_existent_ext = find_with_existent_extension(resolved_no_ext);

		if (with_existent_ext.empty()) {
			throw test_image_does_not_exist{};
		}

		auto final_path = at_stem ;
		final_path += with_existent_ext.extension();

		definition.set_source_path({ final_path.string(), true });

		try {
			load_meta_lua_if_exists(
				lua, 
				definition.meta, 
				image_definition_view({}, definition).get_source_image_path()
			);
		}
		catch (const augs::lua_deserialization_error& err) {
			throw test_scene_asset_loading_error(
				"Failed to load additional properties for %x:\nNot a valid lua table.\n%x",
				at_stem,
				err.what()
			);
		}

		return all_definitions.allocate(std::move(definition));
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

			ensure_eq(id, new_allocation.key);
		}
		catch (const test_image_does_not_exist&) {
			LOG("WARNING! Could not find an official game image: %x", stem);
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