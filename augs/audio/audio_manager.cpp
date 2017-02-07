#include "audio_manager.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/alext.h>

#include "augs/al_log.h"
#include "augs/ensure.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

static std::string list_audio_devices(const ALCchar * const devices) {
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	std::string devices_list;

	devices_list += "Devices list:\n";

	while (device && *device != '\0' && next && *next != '\0') {
		devices_list += typesafe_sprintf("%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}

	return std::move(devices_list);
}

namespace augs {
	void audio_manager::generate_alsoft_ini(const bool hrtf_enabled) {
		std::string alsoft_ini_file;
		alsoft_ini_file += "# Do not modify.";
		alsoft_ini_file += "\n# Hypersomnia generates this file every launch to speak with OpenAL.";
		alsoft_ini_file += "\n# Modification will have no effect.";
		alsoft_ini_file += "\nhrtf = ";
		alsoft_ini_file += hrtf_enabled ? "true" : "false";
		alsoft_ini_file += "\nhrtf-paths = " + augs::get_executable_directory() + "\\hrtf";
		alsoft_ini_file += "\nsources = 2048";

		augs::create_text_file(std::string("alsoft.ini"), alsoft_ini_file);
	}

	audio_manager::audio_manager(const std::string output_device_name) {
		alGetError();

		device = alcOpenDevice(output_device_name.size() > 0 ? output_device_name.c_str() : nullptr);

		context = alcCreateContext(device, nullptr);
		
		if (!context || !make_current()) {
			if (context) {
				alcDestroyContext(context);
			}

			alcCloseDevice(device);
			LOG("\n!!! Failed to set a context !!!\n\n");
		}

		ensure(alcIsExtensionPresent(device, "ALC_EXT_EFX"));

		AL_CHECK(alSpeedOfSound(100.f));
		AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
		AL_CHECK(alListenerf(AL_METERS_PER_UNIT, 1.3f));

		const auto devices = list_audio_devices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));

		LOG(devices);

		augs::create_text_file(std::string("logs/audio_devices.txt"), devices);

		ALint hrtf_status;
		alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtf_status);

		LOG("Default device: %x", alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));
		LOG("HRTF status: %x", hrtf_status);
	}

	bool audio_manager::make_current() {
		return (alcMakeContextCurrent(context)) == ALC_TRUE;
	}

	audio_manager::~audio_manager() {
		AL_CHECK(alcMakeContextCurrent(nullptr));
		alcDestroyContext(context);
		alcCloseDevice(device);
		LOG("Destroyed audio manager");
		context = nullptr;
		device = nullptr;
	}
}