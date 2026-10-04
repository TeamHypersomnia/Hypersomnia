#pragma once
#include "augs/filesystem/path.h"

/*
	Mono buffers can't play through direct channels, which we use for the sounds
	of the listener's own character. With our OpenAL Soft, sound_source plays them direct
	with AL_PANNING_ENABLED_SOFT instead, so mono stays mono: half the memory,
	and a single HRTF convolution for spatialized sounds instead of two.

	The Web's OpenAL has neither direct channels nor source panning.
	A system OpenAL may lack panning or use a different value for this experimental enum.
	There every mono file is duplicated to stereo instead.
*/

#define MONO_TO_STEREO (PLATFORM_WEB || USE_SYSTEM_OPENAL)

namespace augs {
	struct sound_buffer_meta {
		double computed_length_in_seconds = -1.0;
		int channels = 0;

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
