#pragma once
#include "augs/filesystem/path.h"

namespace augs {
	struct sound_buffer_loading_settings {
		// GEN INTROSPECTOR struct augs::sound_buffer_loading_settings
		bool dummy = true;
		// END GEN INTROSPECTOR

		bool operator==(const sound_buffer_loading_settings& b) const {
			return dummy == b.dummy;
		}

		bool operator!=(const sound_buffer_loading_settings& b) const {
			return !operator==(b);
		}
	};

	struct sound_buffer_loading_input {
		const augs::path_type source_sound;
		const sound_buffer_loading_settings settings;
	};
}
