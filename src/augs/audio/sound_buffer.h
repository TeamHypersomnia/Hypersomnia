#pragma once
#include <vector>
#include <optional>

#include "augs/filesystem/path.h"

typedef unsigned int ALuint;
typedef int ALenum;

namespace augs {
	struct sound_data;

	ALenum get_openal_format_of(const sound_data&);

	class single_sound_buffer {
		double computed_length_in_seconds = 0.0;
		ALuint id = 0;
		bool initialized = false;
		
		void set_data(const sound_data&);
		void destroy();
	public:
		single_sound_buffer(const sound_data&);
		~single_sound_buffer();

		single_sound_buffer(single_sound_buffer&& b);
		single_sound_buffer& operator=(single_sound_buffer&& b);

		single_sound_buffer(const single_sound_buffer&) = delete;
		single_sound_buffer& operator=(const single_sound_buffer&) = delete;

		double get_length_in_seconds() const;

		ALuint get_id() const;
		operator ALuint() const;
	};

	struct sound_buffer_loading_input {
		// GEN INTROSPECTOR struct augs::sound_buffer_loading_input
		augs::path_type path_template;
		bool generate_mono = true;
		// END GEN INTROSPECTOR
	};

	class sound_buffer {
		void from_file(const sound_buffer_loading_input);

	public:
		struct variation {
			std::optional<single_sound_buffer> mono;
			std::optional<single_sound_buffer> stereo;

			variation(
				const sound_data&, 
				const bool generate_mono
			);

			const single_sound_buffer& stereo_or_mono() const;
			const single_sound_buffer& mono_or_stereo() const;
		};

		sound_buffer(const sound_buffer_loading_input);

		std::vector<variation> variations;
	};

	struct sound_buffer_logical {
		sound_buffer_logical(const sound_buffer&);

		// GEN INTROSPECTOR struct augs::sound_buffer_logical
		float max_duration_in_seconds = 0.f;
		unsigned num_of_variations = 0xdeadbeef;
		// END GEN INTROSPECTOR
	};
}