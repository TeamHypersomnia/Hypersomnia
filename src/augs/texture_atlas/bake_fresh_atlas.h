#pragma once
#include <unordered_map>
#include <chrono>

#include "augs/filesystem/path.h"
#include "augs/image/font.h"
#include "augs/texture_atlas/atlas_profiler.h"

#include "augs/texture_atlas/loaded_png_vector.h"

using source_image_identifier = augs::path_type;
using source_font_identifier = augs::font_loading_input;

struct atlas_input_subjects {
	std::vector<source_image_identifier> images;
	std::vector<source_font_identifier> fonts;
	loaded_png_vector loaded_images;

	void clear() {
		images.clear();
		fonts.clear();
		loaded_images.clear();
	}

	std::size_t count_images() const {
		return images.size() + loaded_images.size();
	}
};

struct baked_atlas {
	vec2u atlas_image_size;

	std::unordered_map<source_image_identifier, augs::atlas_entry> images;
	std::unordered_map<source_font_identifier, augs::stored_baked_font> fonts;
	std::vector<augs::atlas_entry> loaded_images;

	void clear() {
		atlas_image_size = {};
		images.clear();
		fonts.clear();
	}
};

struct bake_fresh_atlas_input {
	const atlas_input_subjects& subjects;
	const unsigned max_atlas_size;
	const unsigned blitting_threads;
};

struct bake_fresh_atlas_output {
	rgba* const whole_image;
	std::vector<rgba>& fallback_output;

	baked_atlas& baked;

	atlas_profiler& profiler;
};

void bake_fresh_atlas(
	bake_fresh_atlas_input,
	bake_fresh_atlas_output
);