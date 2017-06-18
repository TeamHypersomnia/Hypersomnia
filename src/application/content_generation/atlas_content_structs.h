#pragma once
#include "augs/misc/enum_array.h"

#include "augs/image/font.h"

#include "game/assets/gl_texture_id.h"
#include "game/assets/game_image_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"

using source_image_path = std::string;

struct source_image_loading_input {
	source_image_path path;
	assets::gl_texture_id target_atlas = assets::gl_texture_id::INVALID;
};

using source_font_identifier = augs::font_loading_input;

struct source_font_loading_input {
	source_font_identifier loading_input;
	assets::gl_texture_id target_atlas = assets::gl_texture_id::INVALID;
};

struct atlases_regeneration_output;