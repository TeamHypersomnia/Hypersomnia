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

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
#include "application/setups/editor/official/official_id_to_pool_id.h"

#define OFFICIAL_CONTENT_PATH augs::path_type(OFFICIAL_CONTENT_DIR)

template <class T>
constexpr bool is_pathed_resource_v = is_one_of_v<T, editor_sprite_resource, editor_sound_resource>;

template <class R>
auto& get_resource(const editor_typed_resource_id<R> typed_id, editor_resource_pools& pools) {
	auto& pool = pools.template get_pool_for<R>();
	return pool.get(typed_id.raw).editable;
}

template <class E>
auto& get_resource(const E official_id, editor_resource_pools& pools) {
	return get_resource(to_resource_id(official_id), pools);
}

template <class E>
auto& create_official(const E official_id, editor_resource_pools& pools) {
	using R = to_resource_type_t::at<E>;
	auto& pool = pools.template get_pool_for<R>();

	const auto lowercase_stem = to_lowercase(augs::enum_to_string(official_id));

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

			const auto [new_id, new_object] = pool.allocate(editor_pathed_resource(trimmed_path, hash, stamp));
			const auto recreated_pool_id = to_resource_id(official_id);

			/* Resources must be created in order! */
			ensure(recreated_pool_id.raw == new_id);

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
		const auto [new_id, new_object] = pool.allocate();
		const auto recreated_pool_id = to_resource_id(official_id);

		/* Resources must be created in order! */
		ensure(recreated_pool_id.raw == new_id);

		new_object.unique_name = lowercase_stem;

		return new_object;
	}
}
#endif

#include "application/intercosm.h"
#include "application/setups/editor/official/create_official_sprites.h"
#include "application/setups/editor/official/create_official_materials.h"
#include "application/setups/editor/official/create_official_lights.h"
#include "application/setups/editor/official/create_official_sounds.h"
#include "application/setups/editor/official/create_official_particles.h"
#include "application/setups/editor/official/create_official_weapons.h"

void create_official_resources(
	const intercosm& initial_intercosm,
	editor_resource_pools& pools
) {
	create_lights(initial_intercosm, pools);
	create_materials(initial_intercosm, pools);
	create_particles(initial_intercosm, pools);

	create_sounds(initial_intercosm, pools);

	create_sprites(initial_intercosm, pools);

	create_weapons(initial_intercosm, pools);
}

void create_official_filesystem_from(
	const intercosm& initial_intercosm,
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

			const auto& path = typed_resource.external_file.path_in_project;

			auto& new_node = folder.files[i++];
			new_node.associated_resource = resource_id;
			new_node.set_file_type_by(path.extension().string());

			std::visit(
				[&](const auto& tag) {
					const auto& flavour = initial_intercosm.world.get_flavour(to_entity_flavour_id(tag));
					auto name = flavour.get_name();

					const auto id = str_ops(name).to_lowercase().replace_all(" ", "_").subject;
					new_node.name = id;

					if (auto sprite = flavour.template find<invariants::sprite>()) {
						new_node.custom_thumbnail_path = initial_intercosm.viewables.image_definitions[sprite->image_id].get_source_path().resolve({});
					}

					if (auto animation = flavour.template find<invariants::animation>()) {
						if (animation->id.is_set()) {
							auto path_s = new_node.custom_thumbnail_path.string();
							ensure(ends_with(path_s, "_1.png"));

							path_s.erase(path_s.end() - std::strlen("_1.png"), path_s.end());
							path_s += "_*.png";

							new_node.custom_thumbnail_path = path_s;
						}
					}
				},
				*typed_resource.official_tag
			);
		};

		pool.for_each_id_and_object(resource_handler);

		official_files_root.subfolders.emplace_back(std::move(folder));
	};
	
	handle_pool(resources.get_pool_for<editor_sprite_resource>(), "gfx");
	handle_pool(resources.get_pool_for<editor_sound_resource>(), "sfx");

	official_files_root.adding_children_finished();
}
