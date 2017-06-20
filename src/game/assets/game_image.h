#pragma once
#include <optional>

#include "application/content_generation/atlas_content_structs.h"
#include "application/content_generation/neon_maps.h"
#include "game/detail/convex_partitioned_shape.h"
#include "augs/misc/enum_array.h"
#include "augs/misc/enum_associative_array.h"

class assets_manager;

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
	bool flip_horizontally = false;
	bool flip_vertically = false;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct game_image_definition {
	// GEN INTROSPECTOR struct game_image_definition
	std::string source_image_path;

	std::optional<neon_map_input> neon_map;
	game_image_gui_usage gui_usage;
	bool generate_desaturation = false;
	// END GEN INTROSPECTOR

	std::string get_default_name() const;

	std::string get_neon_map_path() const;
	std::string get_desaturation_path() const;
	
	std::string get_polygonization_path() const;
};

struct game_image_logical_meta {
	vec2u original_image_size;
	convex_partitioned_shape shape;

	vec2u get_size() const {
		return original_image_size;
	}
};

struct game_image_baked {
	augs::enum_array<augs::texture_atlas_entry, texture_map_type> texture_maps;

	std::vector<vec2u> polygonized;
	game_image_gui_usage settings;

	vec2u get_size() const {
		return texture_maps[texture_map_type::DIFFUSE].get_size();
	}

	game_image_logical_meta get_logical_meta(const assets_manager& manager) const;
};
