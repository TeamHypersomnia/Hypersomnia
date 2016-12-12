#include "audio_manager.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

#include "augs/al_log.h"
#include "augs/ensure.h"

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

		device = alcOpenDevice(nullptr);

		context = alcCreateContext(device, nullptr);
		
		if (!context || !make_current()) {
			if (context) {
				alcDestroyContext(context);
			}

			alcCloseDevice(device);
			LOG("\n!!! Failed to set a context !!!\n\n");
		}

		ensure(alcIsExtensionPresent(device, "ALC_EXT_EFX"));

		AL_CHECK(alSpeedOfSound(50.f));
		AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));

		list_audio_devices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));

		LOG("Default device: %x", alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));
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