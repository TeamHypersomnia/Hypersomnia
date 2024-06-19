#pragma once
#include "augs/filesystem/path.h"

namespace augs {
	struct sound_buffer_meta {
		double computed_length_in_seconds = -1.0;

		bool is_set() const {
			return computed_length_in_seconds >= 0.0;
		}
	};

	struct sound_buffer_loading_settings {
		// GEN INTROSPECTOR struct augs::sound_buffer_loading_settings
		bool dummy = true;
		// END GEN INTROSPECTOR

		bool operator==(const sound_buffer_loading_settings& b) const = default;
	};

	struct sound_buffer_loading_input {
		const augs::path_type source_sound;
		const sound_buffer_loading_settings settings;
	};
}
