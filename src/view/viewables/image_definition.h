#pragma once
#include <optional>

#include "augs/filesystem/path.h"

#include "augs/drawing/flip.h"

#include "augs/texture_atlas/atlas_entry.h"

#include "view/viewables/image_meta.h"

#include "view/asset_location_context.h"
#include "view/viewables/regeneration/neon_maps.h"
#include "view/maybe_official_path.h"
#include "view/viewables/asset_definition_view.h"

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

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

	const auto& get_source_path() const {
		return source_image;
	}
};

struct image_definition_view : asset_definition_view<image_definition> {
	using base = asset_definition_view<image_definition>;
	using base::base;

	augs::path_type calc_custom_neon_map_path() const;
	augs::path_type calc_generated_neon_map_path() const;
	augs::path_type calc_desaturation_path() const;

	std::optional<augs::path_type> find_custom_neon_map_path() const;
	std::optional<augs::path_type> find_generated_neon_map_path() const;
	std::optional<augs::path_type> find_desaturation_path() const;

	void regenerate_desaturation(const bool force_regenerate) const;
	void regenerate_neon_map(const bool force_regenerate) const;

	augs::path_type get_source_image_path() const;
	vec2u read_source_image_size() const;
};
