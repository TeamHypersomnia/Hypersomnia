#pragma once
#include <optional>

#include "application/content_generation/atlas_content_structs.h"
#include "application/content_generation/neon_maps.h"
#include "application/content_generation/buttons_with_corners.h"
#include "application/content_generation/scripted_images.h"
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
	vec2 cursor_offset;
	// END GEN INTROSPECTOR
};


struct game_image_definition {
	// GEN INTROSPECTOR struct game_image_definition
	std::optional<std::string> custom_neon_map_path;
	std::optional<neon_map_input> neon_map;
	std::optional<std::vector<vec2u>> physical_shape;
	game_image_gui_usage gui_usage;
	bool generate_desaturation = false;

	std::optional<scripted_image_input> scripted_image;
	// END GEN INTROSPECTOR

	bool should_generate_desaturation() const {
		return generate_desaturation;// generate_desaturation.has_value() && generate_desaturation.value();
	}

	bool has_neon_map() const {
		return custom_neon_map_path.has_value() || neon_map.has_value();
	}

	std::string get_source_image_path(const std::string& from_definition_path) const;
	
	std::string get_neon_map_path(const std::string& from_definition_path) const;
	std::string get_desaturation_path(const std::string& from_definition_path) const;

	std::string get_scripted_image_path(const std::string& from_definition_path) const;
	
	void regenerate_resources(
		const std::string& definition_path, 
		const bool force_regenerate
	) const;

	std::vector<source_image_loading_input> get_atlas_inputs(const std::string& definition_path) const;
};

struct game_image_lua_file {
	std::string path;
	game_image_definition cached;
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
	game_image_gui_usage gui_usage;

	vec2u get_size() const {
		return texture_maps[texture_map_type::DIFFUSE].get_size();
	}

	game_image_logical_meta get_logical_meta(const assets_manager& manager) const;
};
