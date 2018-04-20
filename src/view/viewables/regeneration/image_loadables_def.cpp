#include "augs/image/image.h"
#include "augs/filesystem/file.h"

#include "view/viewables/regeneration/image_loadables_def.h"
#include "view/viewables/regeneration/desaturations.h"

#include "augs/templates/introspection_utils/introspective_equal.h"

augs::path_type get_neon_map_path(augs::path_type from_source_path) {
	return std::string(GENERATED_FILES_DIR) + from_source_path.replace_extension(".neon_map.png").string();
}

augs::path_type get_desaturation_path(augs::path_type from_source_path) {
	return std::string(GENERATED_FILES_DIR) + from_source_path.replace_extension(".desaturation.png").string();
}

augs::path_type image_loadables_def::get_source_image_path() const {
	return source_image_path;
}

std::optional<augs::path_type> image_loadables_def::find_neon_map_path() const {
	if (extras.custom_neon_map_path) {
		return *extras.custom_neon_map_path;
	}
	else if (extras.neon_map) {
		return ::get_neon_map_path(source_image_path);
	}

	return std::nullopt;
}

std::optional<augs::path_type> image_loadables_def::find_desaturation_path() const {
	if (should_generate_desaturation()) {
		return ::get_desaturation_path(source_image_path);
	}

	return std::nullopt;
}

vec2u image_loadables_def::read_source_image_size() const {
	return augs::image::get_size(get_source_image_path());
}

void image_loadables_def::regenerate_all_needed(
	const bool force_regenerate
) const {
	const auto diffuse_path = get_source_image_path();

	if (extras.neon_map) {
		ensure(!extras.custom_neon_map_path.has_value() && "neon_map can't be specified if custom_neon_map_path already is.");
		
		regenerate_neon_map(
			diffuse_path,
			find_neon_map_path().value(),
			extras.neon_map.value(),
			force_regenerate
		);
	}
	
	if (should_generate_desaturation()) {
		regenerate_desaturation(
			diffuse_path,
			find_desaturation_path().value(),
			force_regenerate
		);
	}
}

void image_loadables_def::delete_regenerated_files() const {
	augs::remove_file(::get_neon_map_path(source_image_path));
	augs::remove_file(::get_desaturation_path(source_image_path));
}

bool image_loadables_def::operator==(const image_loadables_def& b) const {
	return augs::introspective_equal(*this, b);
}
