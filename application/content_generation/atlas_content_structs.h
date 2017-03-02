#pragma once
#include "augs/misc/enum_array.h"
#include <unordered_map>

#include "augs/image/font.h"

#include "game/assets/atlas_id.h"
#include "game/assets/texture_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"

typedef std::string source_image_identifier;

struct source_image_loading_input {
	source_image_identifier filename;
	assets::atlas_id target_atlas;
};

typedef augs::font_loading_input source_font_identifier;

struct source_font_loading_input {
	source_font_identifier loading_input;
	assets::atlas_id target_atlas;
};

struct atlases_regeneration_input {
	std::vector<source_font_loading_input> fonts;
	std::vector<source_image_loading_input> images;
};

struct texture_atlas_metadata {
	vec2u atlas_image_size;

	std::unordered_map<source_image_identifier, augs::texture_atlas_entry> images;
	std::unordered_map<source_font_identifier, augs::baked_font> fonts;
};

struct atlases_regeneration_output {
	std::vector<std::pair<assets::atlas_id, texture_atlas_metadata>> metadatas;
};

namespace augs {
	template <class A>
	bool read_object(A& ar, texture_atlas_metadata& data) {
		return
			read_object(ar, data.atlas_image_size)
			&& read_object(ar, data.images)
			&& read_object(ar, data.fonts)
		;
	}

	template <class A>
	void write_object(A& ar, const texture_atlas_metadata& data) {
		write_object(ar, data.atlas_image_size);
		write_object(ar, data.images);
		write_object(ar, data.fonts);
	}
}