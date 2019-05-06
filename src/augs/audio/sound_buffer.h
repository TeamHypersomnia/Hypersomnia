#pragma once
#include <vector>
#include <optional>

#include "augs/audio/sound_buffer_structs.h"

using ALuint = unsigned int;
using ALenum = int;

namespace augs {
	struct sound_data;

	ALenum get_openal_format_of(const sound_data&);

	class single_sound_buffer {
		sound_buffer_meta meta;
		ALuint id = 0;
		bool initialized = false;
		
		void set_data(const sound_data&);
		void destroy();

	public:
		single_sound_buffer(const sound_data&);
		single_sound_buffer(const sound_data&, sound_buffer_loading_settings);

		~single_sound_buffer();

		single_sound_buffer(single_sound_buffer&& b);
		single_sound_buffer& operator=(single_sound_buffer&& b);

		single_sound_buffer(const single_sound_buffer&) = delete;
		single_sound_buffer& operator=(const single_sound_buffer&) = delete;

		double get_length_in_seconds() const;

		ALuint get_id() const;
		operator ALuint() const;

		const auto& get_meta() const {
			return meta;
		}
	};

	class sound_buffer {
		void from_file(const sound_buffer_loading_input);

	public:
		sound_buffer(const sound_buffer_loading_input);
		std::vector<single_sound_buffer> variations;

		const single_sound_buffer& get_buffer(std::size_t variation_index) const;
	};
}