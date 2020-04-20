#pragma once
#include <string>

namespace augs {
	struct audio_volume_settings {
		// GEN INTROSPECTOR struct augs::audio_volume_settings
		float master = 1.f;
		float sound_effects = 1.f;
		float music = 1.f;
		// END GEN INTROSPECTOR

		float get_sound_effects_volume() const {
			return std::clamp(master * sound_effects, 0.f, 1.f);
		}
	};

	struct audio_settings {
		// GEN INTROSPECTOR struct augs::audio_settings
		bool enable_hrtf = false;
		std::string output_device_name = "";
		unsigned max_number_of_sound_sources = 4096u;
		float sound_meters_per_second = 180.f;
		// END GEN INTROSPECTOR
	};
}