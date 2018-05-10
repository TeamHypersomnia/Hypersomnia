#pragma once
#include <unordered_map>
#include <chrono>

#include "augs/filesystem/path.h"
#include "augs/image/font.h"
#include "augs/texture_atlas/atlas_profiler.h"

using source_image_identifier = augs::path_type;
using source_font_identifier = augs::font_loading_input;

struct atlas_input_subjects {
	std::vector<source_image_identifier> images;
	std::vector<source_font_identifier> fonts;

	void clear() {
		images.clear();
		fonts.clear();
	}
};

struct baked_atlas {
	vec2u atlas_image_size;

	std::unordered_map<source_image_identifier, augs::atlas_entry> images;
	std::unordered_map<source_font_identifier, augs::stored_baked_font> fonts;

	void clear() {
		atlas_image_size = {};
		images.clear();
		fonts.clear();
	}
};

struct bake_fresh_atlas_input {
	const atlas_input_subjects& subjects;
	const unsigned max_atlas_size;
};

struct bake_fresh_atlas_output {
	augs::image& whole_image;
	baked_atlas& baked;

	atlas_profiler& profiler;
};

void bake_fresh_atlas(
	bake_fresh_atlas_input,
	bake_fresh_atlas_output
);