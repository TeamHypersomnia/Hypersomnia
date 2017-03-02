#pragma once
#include <map>
#include <experimental\filesystem>

#include "game/assets/texture_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"
#include "augs/image/font.h"

#include "atlas_content_structs.h"

typedef std::experimental::filesystem::file_time_type texture_atlas_image_stamp;
typedef std::experimental::filesystem::file_time_type texture_atlas_font_stamp;

struct texture_atlas_stamp {
	std::unordered_map<source_image_identifier, texture_atlas_image_stamp> image_stamps;
	std::unordered_map<source_font_identifier, texture_atlas_font_stamp> font_stamps;
};

atlases_regeneration_output regenerate_atlases(const atlases_regeneration_input&);

namespace augs {
	template <class A>
	bool read_object(A& ar, texture_atlas_stamp& data) {
		return
			read_object(ar, data.image_stamps)
			&& read_object(ar, data.font_stamps)
		;
	}

	template <class A>
	void write_object(A& ar, const texture_atlas_stamp& data) {
		write_object(ar, data.image_stamps);
		write_object(ar, data.font_stamps);
	}
}

