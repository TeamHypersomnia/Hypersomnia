#pragma once
#include "augs/filesystem/path.h"

namespace augs {
	struct sound_buffer_loading_settings {
		// GEN INTROSPECTOR struct augs::sound_buffer_loading_settings
		bool generate_mono = true;
		// END GEN INTROSPECTOR

		bool operator==(const sound_buffer_loading_settings& b) const {
			return generate_mono == b.generate_mono;
		}
	};

	struct sound_buffer_loading_input {
		const augs::path_type source_sound;
		const sound_buffer_loading_settings settings;
	};
}
