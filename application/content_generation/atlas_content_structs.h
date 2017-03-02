#pragma once
#include "augs/misc/enum_array.h"
#include <unordered_map>

#include "augs/image/font.h"

#include "game/assets/atlas_id.h"
#include "game/assets/texture_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"

struct image_usage_settings {
	struct {
		bool flip_horizontally = false;
		bool flip_vertically = false;
		padding_byte pad[2];

		vec2 bbox_expander;
	} gui;
};

struct source_image_baked {
	vec2u source_size;

	augs::enum_array<augs::texture_atlas_entry, image_map_type> textures;
};

enum class image_map_type {
	DIFFUSE,
	NEON,
	DESATURATED,

	COUNT
};

struct source_image_location {
	std::string filename;
	assets::atlas_id target_atlas;
};

struct source_font_location {
	augs::font_loading_input loading_input;
	assets::atlas_id target_atlas;
};

struct all_information_about_image {
	augs::enum_array<source_image_location, image_map_type> textures;

	std::string polygonization_filename;
	image_usage_settings settings;
};

typedef source_font_location all_information_about_font;

struct atlas_regeneration_input {
	std::vector<source_font_location> fonts;
	std::vector<source_image_location> images;
};


/*
struct source_image_input {
	augs::enum_array<std::string, image_map_type> textures;
};

typedef augs::font_loading_input source_font_input;

struct requested_atlas_resources {
	std::unordered_map<assets::texture_id, source_image_input> images;
	std::unordered_map<assets::font_id, source_font_input> fonts;

	void request(
		const assets::texture_id,
		const std::string& diffuse_filename
	);

	void request(
		const assets::font_id,
		const augs::font_loading_input in
	);

	void request_indexed(
		const assets::texture_id first,
		const assets::texture_id last,
		const std::string& diffuse_filename
	);

	void request_button_with_corners(
		const assets::texture_id inside_image_id,
		const std::string& first_filename
	);

	void request_desaturated(const assets::texture_id);
	void request_neon_map(const assets::texture_id);
};
*/