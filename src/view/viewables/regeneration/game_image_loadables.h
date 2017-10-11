#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/misc/enum/enum_associative_array.h"
#include "augs/filesystem/path.h"

#include "augs/drawing/flip.h"

#include "augs/texture_atlas/texture_atlas_entry.h"

#include "view/viewables/regeneration/neon_maps.h"

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

struct game_image_extra_loadables {
	// GEN INTROSPECTOR struct game_image_extra_loadables
	std::optional<augs::path_type> custom_neon_map_path;
	std::optional<neon_map_input> neon_map;
	bool generate_desaturation = false;
	// END GEN INTROSPECTOR
};

struct game_image_loadables {
	// GEN INTROSPECTOR struct game_image_loadables
	augs::path_type source_image_path;
	game_image_extra_loadables extras;
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
	std::optional<augs::path_type> get_neon_map_path() const;
	std::optional<augs::path_type> get_desaturation_path() const;

	mutable vec2u cached_source_image_size;
	vec2u get_size() const;
};
