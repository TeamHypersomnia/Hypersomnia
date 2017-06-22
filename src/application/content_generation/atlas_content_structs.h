#pragma once
#include "augs/misc/enum_array.h"

#include "augs/image/font.h"

#include "game/assets/gl_texture_id.h"
#include "game/assets/game_image_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"

using source_image_identifier = std::string;

struct source_font_identifier {
	// GEN INTROSPECTOR struct source_font_identifier
	std::string path;
	augs::font_loading_input input;
	// END GEN INTROSPECTOR

	bool operator==(const source_font_identifier& b) const {
		return path == b.path && input == b.input;
	}
};

namespace std {
	template <>
	struct hash<source_font_identifier> {
		size_t operator()(const source_font_identifier& in) const {
			return augs::simple_two_hash(in.path, in.input);
		}
	};
}

struct source_image_loading_input {
	source_image_identifier path;
	assets::gl_texture_id target_atlas = assets::gl_texture_id::INVALID;
};

struct source_font_loading_input {
	// GEN INTROSPECTOR struct source_font_loading_input
	source_font_identifier loading_input;
	assets::gl_texture_id target_atlas = assets::gl_texture_id::INVALID;
	// END GEN INTROSPECTOR
};

struct atlases_regeneration_output;