#include "augs/image/image.h"
#include "augs/string/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/graphics/gore_colors.h"

#include "view/viewables/image_definition.h"
#include "view/viewables/regeneration/desaturations.h"

augs::path_type get_path_in_cache(const augs::path_type& from_source_path) {
	/*
		The result must always stay within CACHE_DIR.

		Note the source might come as an absolute path, e.g. resolved
		through a symlink - a naive relative() against USER_DIR would then
		produce a ..-ridden path escaping the cache, polluting the source's
		own folder with the generated maps and their stamps.
	*/

	if (::begins_with(from_source_path.string(), OFFICIAL_CONTENT_DIR.string())) {
		return CACHE_DIR / from_source_path;
	}

	auto is_within = [](const augs::path_type& p, const augs::path_type& root) {
		const auto rel = p.lexically_relative(root);

		return !rel.empty() && *rel.begin() != "..";
	};

	std::error_code ec;

	const auto source = std::filesystem::weakly_canonical(from_source_path, ec);

	if (!ec) {
		const auto official = std::filesystem::weakly_canonical(OFFICIAL_CONTENT_DIR, ec);

		if (!ec && is_within(source, official)) {
			return CACHE_DIR / OFFICIAL_CONTENT_FOLDER_NAME / source.lexically_relative(official);
		}

		const auto user = std::filesystem::weakly_canonical(USER_DIR, ec);

		if (!ec && is_within(source, user)) {
			return CACHE_DIR / source.lexically_relative(user);
		}
	}

	/* A foreign source: flatten it inside the cache, never escaping it. */

	auto flattened = augs::path_type("foreign");

	for (const auto& part : augs::path_type(from_source_path).relative_path()) {
		if (part != ".." && part != ".") {
			flattened /= part;
		}
	}

	return CACHE_DIR / flattened;
}

augs::path_type get_neon_map_path(augs::path_type from_source_path) {
	return get_path_in_cache(from_source_path).replace_extension(".neon_map.png").string();
}

augs::path_type get_desaturation_path(augs::path_type from_source_path) {
	return get_path_in_cache(from_source_path).replace_extension(".desaturation.png").string();
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

#if !HEADLESS
std::optional<cached_neon_map_in> image_definition_view::should_regenerate_neon_map(
	const bool force_regenerate,
	const bool gore_enabled
) const {
	if (const auto generated_neon_map_path = find_generated_neon_map_path()) {
		const auto diffuse_path = resolved_source_path;

		const bool remap_gore = !gore_enabled && augs::path_matches_gore_words(diffuse_path.string());

		auto neon_input = get_def().meta.extra_loadables.generate_neon_map.value;

		if (remap_gore) {
			augs::remap_gore_light_colors(neon_input.light_colors);
		}

		return ::should_regenerate_neon_map(
			diffuse_path,
			*generated_neon_map_path,
			neon_input,
			force_regenerate,
			remap_gore
		);
	}

	return std::nullopt;
}

void image_definition_view::regenerate_neon_map(
	const cached_neon_map_in& cached_in,
	const bool gore_enabled
) const {
	const auto diffuse_path = resolved_source_path;

	if (const auto generated_neon_map_path = find_generated_neon_map_path()) {
		auto neon_input = get_def().meta.extra_loadables.generate_neon_map.value;

		const bool remap_gore = !gore_enabled && augs::path_matches_gore_words(diffuse_path.string());

		if (remap_gore) {
			augs::remap_gore_light_colors(neon_input.light_colors);
		}

		::regenerate_neon_map(
			diffuse_path,
			*generated_neon_map_path,
			neon_input,
			cached_in,
			remap_gore
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
#endif
