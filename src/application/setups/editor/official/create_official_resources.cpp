#include "application/setups/editor/official/create_official_resources.h"
#include "augs/string/string_templates.h"
#include "augs/templates/type_map.h"
#include "augs/templates/introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/filesystem/find_path.h"
#include "augs/templates/folded_finders.h"
#include "application/setups/editor/editor_filesystem.h"
#include "augs/image/image.h"

#define OFFICIAL_CONTENT_PATH augs::path_type(OFFICIAL_CONTENT_DIR)

template <class T>
constexpr bool is_pathed_resource_v = is_one_of_v<T, editor_sprite_resource, editor_sound_resource>;

using to_resource_type_t = type_map<
	type_pair<official_sprites, editor_sprite_resource>,
	type_pair<official_sounds, editor_sound_resource>,
	type_pair<official_lights, editor_light_resource>
>;

template <class E>
auto& create_official(E id, editor_resource_pools& pools) {
	using R = to_resource_type_t::at<E>;
	auto& pool = pools.template get_pool_for<R>();

	const auto lowercase_stem = to_lowercase(augs::enum_to_string(id));

	if constexpr(is_pathed_resource_v<R>) {
		auto create_pathed_resource = [&](
			augs::path_type content_folder,
			const auto exts,
			const auto default_ext
		) -> decltype(auto) {
			const auto full_path_no_ext = OFFICIAL_CONTENT_PATH / content_folder / lowercase_stem;
			const auto found_extension = augs::first_existing_extension(full_path_no_ext, exts, default_ext).extension();

			/* Due to how maybe_official_path is handled, we have to remove the first element */
			auto trimmed_path = content_folder / lowercase_stem;
			trimmed_path += found_extension;

			const auto hash = "";
			const auto stamp = augs::file_time_type();

			auto& new_object = pool.allocate(editor_pathed_resource(trimmed_path, hash, stamp)).object;

			return new_object;
		};

		if constexpr(std::is_base_of_v<R, editor_sprite_resource>) {
			auto& result = create_pathed_resource("gfx", augs::IMAGE_EXTENSIONS, ".png");

			const auto full_path = result.external_file.path_in(OFFICIAL_CONTENT_PATH);

			try {
				result.editable.size = augs::image::get_size(full_path);
			}
			catch (...) {
				result.editable.size.set(32, 32);
			}

			return result;
		}
		else if constexpr(std::is_base_of_v<R, editor_sound_resource>) {
			return create_pathed_resource("sfx", augs::SOUND_EXTENSIONS, ".wav");
		}
		else {
			static_assert(always_false_v<R>);
		}
	}
	else {
		auto& new_object = pool.allocate().object;
		new_object.unique_name = lowercase_stem;

		return new_object;
	}
}

void create_lights(editor_resource_pools& pools) {
	{
		auto& strong_lamp = create_official(official_lights::STRONG_LAMP, pools);
		(void)strong_lamp;
	}

	{
		auto& aquarium_lamp = create_official(official_lights::AQUARIUM_LAMP, pools).editable;
		aquarium_lamp.attenuation.constant = 75;
		aquarium_lamp.attenuation.quadratic = 631;
	}
}

void create_sprites(editor_resource_pools& pools) {
	{
		auto& road = create_official(official_sprites::ROAD, pools);
		(void)road;
	}
}

void create_official_resources(editor_resource_pools& pools) {
	create_lights(pools);

	create_sprites(pools);
}

void create_official_filesystem_from(
	editor_resource_pools& resources,
	editor_filesystem_node& official_files_root
) {
	official_files_root.clear();

	auto handle_pool = [&]<typename P>(const P& pool, const auto content_folder_name) {
		using resource_type = typename P::value_type;

		editor_filesystem_node folder;
		folder.name = content_folder_name;
		folder.type = editor_filesystem_node_type::FOLDER;

		folder.files.resize(pool.size());
		std::size_t i = 0;

		auto resource_handler = [&](const auto raw_id, const resource_type& typed_resource) {
			editor_resource_id resource_id;

			resource_id.is_official = true;
			resource_id.raw = raw_id;
			resource_id.type_id.set<resource_type>();

			auto& new_node = folder.files[i++];
			new_node.associated_resource = resource_id;
			new_node.name = typed_resource.external_file.path_in_project.filename();
			new_node.type = editor_filesystem_node_type::OTHER_RESOURCE;
		};

		pool.for_each_id_and_object(resource_handler);

		official_files_root.subfolders.emplace_back(std::move(folder));
	};
	
	handle_pool(resources.get_pool_for<editor_sprite_resource>(), "gfx");
	handle_pool(resources.get_pool_for<editor_sound_resource>(), "sfx");

	official_files_root.adding_children_finished();
}
