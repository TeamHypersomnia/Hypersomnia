#include "application/setups/editor/official/create_official_resources.h"
#include "augs/string/string_templates.h"
#include "augs/templates/type_map.h"
#include "augs/templates/introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/filesystem/find_path.h"
#include "augs/templates/folded_finders.h"

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
		augs::path_type content_folder;
		std::string default_ext;

		auto exts = augs::IMAGE_EXTENSIONS;

		if constexpr(std::is_base_of_v<R, editor_sprite_resource>) {
			content_folder = "gfx";
			default_ext = ".png";

		}
		else if constexpr(std::is_base_of_v<R, editor_sound_resource>) {
			content_folder = "sfx";
			default_ext = ".wav";

			exts = augs::SOUND_EXTENSIONS;
		}
		else {
			static_assert(always_false_v<R>);
		}

		const auto parent_folder = augs::path_type(OFFICIAL_CONTENT_DIR);
		const auto full_path_no_ext = parent_folder / content_folder / lowercase_stem;
		const auto found_extension = augs::first_existing_extension(full_path_no_ext, exts, default_ext).extension();

		/* Due to how maybe_official_path is handled, we have to remove the first element */
		auto trimmed_path = content_folder / lowercase_stem;
		trimmed_path += found_extension;

		const auto hash = "";
		const auto stamp = augs::file_time_type();

		auto& new_object = pool.allocate(editor_pathed_resource(trimmed_path, hash, stamp)).object;

		return new_object;
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
