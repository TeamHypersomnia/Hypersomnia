#pragma once
#include <optional>

#include "augs/misc/convex_partitioned_shape.h"
#include "augs/misc/enum_array.h"
#include "augs/misc/enum_associative_array.h"
#include "augs/drawing/flip.h"

#include "augs/texture_atlas/texture_atlas_entry.h"

#include "game/assets/assets_declarations.h"
#include "game/assets/game_image_id.h"

#include "application/content_regeneration/neon_maps.h"

enum class texture_map_type {
	// GEN INTROSPECTOR enum class texture_map_type
	DIFFUSE,
	NEON,
	DESATURATED,

	COUNT
	// END GEN INTROSPECTOR
};

struct game_image_gui_usage {
	// GEN INTROSPECTOR struct game_image_gui_usage
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

struct game_image_definition {
	// GEN INTROSPECTOR struct game_image_definition
	augs::path_type source_image_path;

	std::optional<augs::path_type> custom_neon_map_path;
	std::optional<neon_map_input> neon_map;
	std::optional<std::vector<vec2u>> physical_shape;
	game_image_gui_usage gui_usage;
	bool generate_desaturation = false;
#if TODO
	bool force_always_loaded = false;
#endif
	// END GEN INTROSPECTOR

	vec2u get_size() const;

	bool should_generate_desaturation() const {
		return generate_desaturation;
	}

	bool has_neon_map() const { 
		return custom_neon_map_path.has_value() || neon_map.has_value();
	}

	void regenerate_all_needed(const bool force_regenerate) const;

	augs::path_type get_source_image_path() const;
	std::optional<augs::path_type> get_neon_map_path() const;
	std::optional<augs::path_type> get_desaturation_path() const;
};

struct game_image_logical {
	vec2u original_image_size;
	convex_partitioned_shape shape;

	game_image_logical(const game_image_definition&);
	
	vec2u get_size() const {
		return original_image_size;
	}
};

struct game_image_in_atlas {
	augs::enum_array<augs::texture_atlas_entry, texture_map_type> texture_maps;

	operator augs::texture_atlas_entry() const {
		return texture_maps[texture_map_type::DIFFUSE];
	}

	vec2u get_size() const {
		return texture_maps[texture_map_type::DIFFUSE].get_original_size();
	}
};