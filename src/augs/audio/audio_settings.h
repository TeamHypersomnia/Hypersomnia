#pragma once
#include <string>

enum class audio_output_mode {
	// GEN INTROSPECTOR enum class audio_output_mode
	AUTO,
#if !PLATFORM_WEB
	STEREO_BASIC,
	STEREO_UHJ,
#endif
	STEREO_HRTF,
#if !PLATFORM_WEB
	SURROUND_5_1,
	SURROUND_6_1,
	SURROUND_7_1,
#endif
	COUNT
	// END GEN INTROSPECTOR
};

template <class Enum>
auto format_enum(const Enum e);

template <>
inline auto format_enum(const audio_output_mode e) {
	constexpr std::array<const char*, 7> vals = {
#if PLATFORM_WEB
		"Auto",
		"Stereo HRTF (Headphones)",
#else
		"Auto",
		"Stereo (Basic)",
		"Stereo (Ambisonic UHJ)",
		"Stereo HRTF (Headphones)",
		"5.1 Surround",
		"6.1 Surround",
		"7.1 Surround"
#endif
	};

	if (static_cast<uint8_t>(e) < static_cast<uint8_t>(vals.size())) {
		return std::string(vals[static_cast<uint8_t>(e)]);
	}

	return std::string("UnknownEnumValue");
}

namespace augs {
	float convert_audio_volume(float);

	struct audio_volume_settings {
		// GEN INTROSPECTOR struct augs::audio_volume_settings
		float master = 1.f;
		float sound_effects = 1.f;
		float music = 1.f;
		// END GEN INTROSPECTOR

		float get_sound_effects_volume() const {
			return convert_audio_volume(std::clamp(master * sound_effects, 0.f, 1.f));
		}

		float get_music_volume() const {
			return convert_audio_volume(std::clamp(master * music, 0.f, 1.f));
		}

		bool operator==(const audio_volume_settings& b) const = default;
	};

	struct audio_settings {
		// GEN INTROSPECTOR struct augs::audio_settings
		bool mute_main_menu_background = false;
		audio_output_mode output_mode = audio_output_mode::STEREO_HRTF;
		std::string output_device_name = "";
		unsigned max_number_of_sound_sources = 4096u;
		float sound_meters_per_second = 180.f;
		// END GEN INTROSPECTOR

		bool operator==(const audio_settings& b) const = default;
	};
}