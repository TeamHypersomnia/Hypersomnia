#include "augs/image/image.h"
#include "augs/string/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/lua_file.h"

#include "view/viewables/image_definition.h"
#include "view/viewables/regeneration/desaturations.h"

augs::path_type get_neon_map_path(augs::path_type from_source_path) {
	return augs::path_type(GENERATED_FILES_DIR) / from_source_path.relative_path().replace_extension(".neon_map.png").string();
}

augs::path_type get_desaturation_path(augs::path_type from_source_path) {
	return augs::path_type(GENERATED_FILES_DIR) / from_source_path.relative_path().replace_extension(".desaturation.png").string();
}

augs::path_type image_definition_view::calc_custom_neon_map_path() const {
	return augs::path_type(resolved_source_path).replace_extension(".neon_map.png");
}

augs::path_type image_definition_view::calc_generated_neon_map_path() const {
	return ::get_neon_map_path(resolved_source_path);
}

augs::path_type image_definition_view::calc_desaturation_path() const {
	return ::get_desaturation_path(resolved_source_path);
}

std::optional<augs::path_type> image_definition_view::find_custom_neon_map_path() const {
	if (const auto p = calc_custom_neon_map_path();
   		augs::exists(p)
	) {
		return p;
	}

	return std::nullopt;
}

std::optional<augs::path_type> image_definition_view::find_generated_neon_map_path() const {
	if (get_def().meta.extra_loadables.should_generate_neon_map()) {
		return calc_generated_neon_map_path();
	}

	return std::nullopt;
}

std::optional<augs::path_type> image_definition_view::find_desaturation_path() const {
	if (get_def().meta.extra_loadables.should_generate_desaturation()) {
		return calc_desaturation_path();
	}

	return std::nullopt;
}

augs::path_type image_definition_view::get_source_image_path() const {
	return resolved_source_path;
}

vec2u image_definition_view::read_source_image_size() const {
	return augs::image::get_size(resolved_source_path);
}

std::optional<cached_neon_map_in> image_definition_view::should_regenerate_neon_map(
	const bool force_regenerate
) const {
	if (const auto generated_neon_map_path = find_generated_neon_map_path()) {
		const auto diffuse_path = resolved_source_path;

		return ::should_regenerate_neon_map(
			diffuse_path,
			*generated_neon_map_path,
			get_def().meta.extra_loadables.generate_neon_map.value,
			force_regenerate
		);
	}

	return std::nullopt;
}

void image_definition_view::regenerate_neon_map(
	const cached_neon_map_in& cached_in
) const {
	const auto diffuse_path = resolved_source_path;

	if (const auto generated_neon_map_path = find_generated_neon_map_path()) {
		::regenerate_neon_map(
			diffuse_path,
			*generated_neon_map_path,
			get_def().meta.extra_loadables.generate_neon_map.value,
			cached_in
		);
	}
}

void image_definition_view::regenerate_desaturation(
	const bool force_regenerate
) const {
	const auto diffuse_path = resolved_source_path;

	if (const auto desaturation_path = find_desaturation_path()) {
		::regenerate_desaturation(
			diffuse_path,
			*desaturation_path,
			force_regenerate
		);
	}
}
