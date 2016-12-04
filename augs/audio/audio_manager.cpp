#include "audio_manager.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "augs/log.h"

namespace augs {
	audio_manager::audio_manager() {
		device = alcOpenDevice(nullptr);

		context = alcCreateContext(device, nullptr);

		if (!context || !make_current()) {
			if (context) {
				alcDestroyContext(context);
			}

			alcCloseDevice(device);
			LOG("\n!!! Failed to set a context !!!\n\n");
		}
	}

	bool audio_manager::make_current() {
		return alcMakeContextCurrent(context) == ALC_TRUE;
	}

	audio_manager::~audio_manager() {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
}