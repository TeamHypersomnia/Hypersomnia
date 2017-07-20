#pragma once
#include <unordered_map>
#include <chrono>

#include "game/assets/game_image_id.h"
#include "game/assets/font_id.h"

#include "augs/pad_bytes.h"
#include "augs/image/font.h"

#include "atlas_content_structs.h"

struct atlases_regeneration_input {
	std::vector<source_font_loading_input> fonts;
	std::vector<source_image_loading_input> images;
};

using texture_atlas_image_stamp = std::chrono::system_clock::time_point;
using texture_atlas_font_stamp = std::chrono::system_clock::time_point;

struct texture_atlas_stamp {
	// GEN INTROSPECTOR struct texture_atlas_stamp
	std::unordered_map<source_image_identifier, texture_atlas_image_stamp> image_stamps;
	std::unordered_map<source_font_identifier, texture_atlas_font_stamp> font_stamps;
	// END GEN INTROSPECTOR
};

struct texture_atlas_metadata {
	// GEN INTROSPECTOR struct texture_atlas_metadata
	vec2u atlas_image_size;

	std::unordered_map<source_image_identifier, augs::texture_atlas_entry> images;
	std::unordered_map<source_font_identifier, augs::baked_font> fonts;
	// END GEN INTROSPECTOR
};

struct atlases_regeneration_output {
	std::vector<std::pair<assets::gl_texture_id, texture_atlas_metadata>> metadatas;
};

atlases_regeneration_output regenerate_atlases(
	const atlases_regeneration_input&,
	const bool force_regenerate,
	const bool always_check_source_images_integrity,
	const bool save_atlases_as_binary,
	const unsigned packer_detail_max_atlas_size
);
