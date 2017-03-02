#pragma once
#include <map>
#include <experimental\filesystem>

#include "game/assets/texture_id.h"
#include "game/assets/font_id.h"

#include "augs/padding_byte.h"
#include "augs/image/font.h"

#include "atlas_content_structs.h"

struct texture_atlas_image_stamp {
	std::experimental::filesystem::file_time_type last_write_time;
};

struct texture_atlas_font_stamp {
	std::experimental::filesystem::file_time_type last_write_time;
	augs::font_loading_input loading_input;
};

struct texture_atlas_stamp {
	std::map<std::string, texture_atlas_image_stamp> image_stamps;
	std::map<std::string, texture_atlas_font_stamp> font_stamps;
};

atlases_regeneration_output regenerate_atlases(const atlases_regeneration_input&);

namespace augs {
	template <class A>
	bool read_object(A& ar, texture_atlas_font_stamp& data) {
		return
			read_object(ar, data.source_filename)
			&& read_object(ar, data.neon_map_filename)
			&& read_object(ar, data.source_timestamp)
			&& read_object(ar, data.neon_map_timestamp)
			&& read_object(ar, data.should_generate_desaturated)
		;
	}

	template <class A>
	void write_object(A& ar, const texture_atlas_image_input_entry& data) {
		write_object(ar, data.source_filename);
		write_object(ar, data.neon_map_filename);
		write_object(ar, data.source_timestamp);
		write_object(ar, data.neon_map_timestamp);
		write_object(ar, data.should_generate_desaturated);
	}

	template <class A>
	bool read_object(A& ar, texture_atlas_input_stamp& data) {
		return
			read_object(ar, data.images)
			&& read_object(ar, data.fonts)
		;
	}

	template <class A>
	void write_object(A& ar, const texture_atlas_input_stamp& data) {
		write_object(ar, data.images);
		write_object(ar, data.fonts);
	}
}

