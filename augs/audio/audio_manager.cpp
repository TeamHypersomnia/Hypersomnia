#include "audio_manager.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "augs/al_log.h"

static void list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	LOG("Devices list:\n");
	LOG("----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		LOG("%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	LOG("----------\n");
}

namespace augs {
	audio_manager::audio_manager() {
		alGetError();

		device = alcOpenDevice("Generic Software on Speakers (Realtek High Definition Audio)");

		context = alcCreateContext(device, nullptr);

		if (!context || !make_current()) {
			if (context) {
				alcDestroyContext(context);
			}

			alcCloseDevice(device);
			LOG("\n!!! Failed to set a context !!!\n\n");
		}

		list_audio_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));

		LOG("Default device: %x", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
	}

	bool audio_manager::make_current() {
		return AL_CHECK(alcMakeContextCurrent(context)) == ALC_TRUE;
	}

	audio_manager::~audio_manager() {
		AL_CHECK(alcMakeContextCurrent(nullptr));
		alcDestroyContext(context);
		alcCloseDevice(device);

		context = nullptr;
		device = nullptr;
	}
}