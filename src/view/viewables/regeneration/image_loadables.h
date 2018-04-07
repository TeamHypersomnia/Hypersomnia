#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/filesystem/path.h"

#include "augs/drawing/flip.h"

#include "augs/texture_atlas/texture_atlas_entry.h"

#include "view/viewables/regeneration/neon_maps.h"

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

struct image_extra_loadables {
	// GEN INTROSPECTOR struct image_extra_loadables
	std::optional<augs::path_type> custom_neon_map_path;
	std::optional<neon_map_input> neon_map;
	bool generate_desaturation = false;
	// END GEN INTROSPECTOR
};

struct image_loadables {
	// GEN INTROSPECTOR struct image_loadables
	augs::path_type source_image_path;
	image_extra_loadables extras;
	// END GEN INTROSPECTOR

	bool should_generate_desaturation() const {
		return extras.generate_desaturation;
	}

	bool has_neon_map() const { 
		return 
			extras.custom_neon_map_path.has_value()
			|| extras.neon_map.has_value()
		;
	}

	void regenerate_all_needed(const bool force_regenerate) const;

	augs::path_type get_source_image_path() const;
	std::optional<augs::path_type> find_neon_map_path() const;
	std::optional<augs::path_type> find_desaturation_path() const;

	vec2u read_source_image_size() const;
};
