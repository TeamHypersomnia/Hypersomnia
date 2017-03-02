#pragma once
#include "augs/misc/enum_array.h"
#include <unordered_map>

#include "augs/image/font.h"

#include "game/assets/atlas_id.h"
#include "game/assets/texture_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"

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
		padding_byte pad[2];

		vec2 bbox_expander;
	} gui;
};

typedef std::string source_image_identifier;

struct source_image_loading_input {
	source_image_identifier filename;
	assets::atlas_id target_atlas;
};

struct game_image_request {
	augs::enum_array<source_image_loading_input, texture_map_type> texture_maps;

	std::string polygonization_filename;
	game_image_usage_settings settings;
};

typedef std::unordered_map<assets::texture_id, game_image_request> game_image_requests;

struct game_image_baked {
	augs::enum_array<augs::texture_atlas_entry, texture_map_type> texture_maps;

	std::vector<vec2i> polygonized;
	game_image_usage_settings settings;
};

typedef augs::font_loading_input source_font_identifier;

struct source_font_loading_input {
	source_font_identifier loading_input;
	assets::atlas_id target_atlas;
};

typedef source_font_loading_input game_font_request;
typedef std::unordered_map<assets::font_id, game_font_request> game_font_requests;

typedef augs::font_metadata game_font_baked;

struct atlases_regeneration_input {
	std::vector<source_font_loading_input> fonts;
	std::vector<source_image_loading_input> images;
};

struct texture_atlas_metadata {
	std::unordered_map<source_image_identifier, augs::texture_atlas_entry> images;
	std::unordered_map<source_font_identifier, augs::font_metadata> fonts;
};

struct atlases_regeneration_output {
	std::vector<std::pair<assets::atlas_id, texture_atlas_metadata>> metadatas;
};