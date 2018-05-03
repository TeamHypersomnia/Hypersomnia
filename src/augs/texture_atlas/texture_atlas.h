#pragma once
#include <unordered_map>
#include <chrono>

#include "augs/filesystem/path.h"
#include "augs/image/font.h"
#include "augs/texture_atlas/texture_atlas_profiler.h"

using source_image_identifier = augs::path_type;
using source_font_identifier = augs::font_loading_input;

struct atlas_regeneration_subjects {
	std::vector<source_image_identifier> images;
	std::vector<source_font_identifier> fonts;

	void clear() {
		images.clear();
		fonts.clear();
	}
};

struct atlas_regeneration_settings {
	const unsigned packer_detail_max_atlas_size;
	const augs::path_type regeneration_path;

	const bool force_regenerate;
	const bool skip_source_image_integrity_check;
};

using texture_atlas_image_stamp = std::chrono::system_clock::time_point;
using texture_atlas_font_stamp = std::chrono::system_clock::time_point;

struct texture_atlas_stamp {
	// GEN INTROSPECTOR struct texture_atlas_stamp
	std::unordered_map<source_image_identifier, texture_atlas_image_stamp> image_stamps;
	std::unordered_map<source_font_identifier, texture_atlas_font_stamp> font_stamps;
	// END GEN INTROSPECTOR
};

struct regenerated_atlas_input {
	const atlas_regeneration_subjects& subjects;
	const atlas_regeneration_settings settings;
	augs::image& output_image;
	atlas_profiler& profiler;
};

struct regenerated_atlas {
	// GEN INTROSPECTOR struct regenerated_atlas
	vec2u atlas_image_size;

	std::unordered_map<source_image_identifier, augs::texture_atlas_entry> baked_images;
	std::unordered_map<source_font_identifier, augs::stored_baked_font> stored_baked_fonts;
	// END GEN INTROSPECTOR

	regenerated_atlas(regenerated_atlas_input);
};