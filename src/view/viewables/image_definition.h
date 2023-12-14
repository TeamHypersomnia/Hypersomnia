#pragma once
#include <optional>

#include "augs/filesystem/path.h"

#include "augs/drawing/flip.h"

#include "augs/texture_atlas/atlas_entry.h"

#include "view/viewables/image_meta.h"

#include "view/viewables/regeneration/neon_maps.h"
#include "view/maybe_official_path_types.h"
#include "view/viewables/asset_definition_view.h"

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

namespace sol {
	class state;
}

augs::path_type get_path_in_cache(const augs::path_type& from_source_path);

struct image_definition {
	// GEN INTROSPECTOR struct image_definition
	maybe_official_image_path source_image;
	image_meta meta;
	// END GEN INTROSPECTOR

	bool loadables_differ(const image_definition& b) const {
		return 
			source_image != b.source_image 
			|| meta.extra_loadables != b.meta.extra_loadables
		;
	}

	void set_source_path(const maybe_official_image_path& p) {
		source_image = p;
	}

	std::optional<augs::path_type> get_source_gif_path() const {
		auto path = source_image.path;

		if (path.extension() == ".png") {
			if (path.replace_extension("").replace_extension("").extension() == ".gif") {
				return path;
			}
		}

		return std::nullopt;
	};

	auto get_specified_source_path() const {
		return source_image;
	}

	auto get_loadable_path() const {
		const bool is_generated_in_cache = get_source_gif_path() != std::nullopt;

		if (is_generated_in_cache) {
			auto path = source_image;
			path.path = ::get_path_in_cache(path.path);

			return path;
		}

		return get_specified_source_path();
	}
};

struct image_definition_view : asset_definition_view<const image_definition> {
	using base = asset_definition_view<const image_definition>;
	using base::base;

	augs::path_type calc_custom_neon_map_path() const;
	augs::path_type calc_generated_neon_map_path() const;
	augs::path_type calc_desaturation_path() const;

	std::optional<augs::path_type> find_custom_neon_map_path() const;
	std::optional<augs::path_type> find_generated_neon_map_path() const;
	std::optional<augs::path_type> find_desaturation_path() const;

	void regenerate_desaturation(const bool force_regenerate) const;

	std::optional<cached_neon_map_in> should_regenerate_neon_map(const bool force_regenerate) const;
	void regenerate_neon_map(const cached_neon_map_in&) const;

	augs::path_type get_source_image_path() const;
	vec2u read_source_image_size() const;
};
