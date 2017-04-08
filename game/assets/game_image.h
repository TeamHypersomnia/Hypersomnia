#pragma once
#include "application/content_generation/atlas_content_structs.h"

#include "game/detail/shape_variant.h"
#include "augs/misc/enum_array.h"

enum class texture_map_type {
	DIFFUSE,
	NEON,
	DESATURATED,

	COUNT
};

struct game_image_usage_settings {
	struct {
		bool flip_horizontally = false;
		bool flip_vertically = false;
		std::array<padding_byte, 2> pad;

		vec2 bbox_expander;
	} gui;
};

struct game_image_request {
	augs::enum_array<source_image_loading_input, texture_map_type> texture_maps;

	std::string polygonization_filename;
	game_image_usage_settings settings;
};

struct game_image_logical_meta {
	vec2u original_image_size;
	shape_variant shape;

	vec2u get_size() const {
		return original_image_size;
	}
};

struct game_image_baked {
	augs::enum_array<augs::texture_atlas_entry, texture_map_type> texture_maps;

	std::vector<vec2u> polygonized;
	game_image_usage_settings settings;

	vec2u get_size() const {
		return texture_maps[texture_map_type::DIFFUSE].get_size();
	}

	game_image_logical_meta get_logical_meta() const;
};
