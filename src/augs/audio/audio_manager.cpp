#include "audio_manager.h"

#include "augs/al_log.h"

#if BUILD_OPENAL
extern "C" {
	#include <compat.h>
}

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/alext.h>
#endif

#include "augs/ensure.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#if BUILD_OPENAL
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

	return devices_list;
}
#endif

namespace augs {
	void audio_manager::generate_alsoft_ini(
		const bool hrtf_enabled,
		const unsigned max_number_of_sound_sources
	) {
		std::string alsoft_ini_file;
		alsoft_ini_file += "# Do not modify.";
		alsoft_ini_file += "\n# Hypersomnia generates this file every launch to speak with OpenAL.";
		alsoft_ini_file += "\n# Modification will have no effect.";
		alsoft_ini_file += "\nhrtf = ";
		alsoft_ini_file += hrtf_enabled ? "true" : "false";
		alsoft_ini_file += "\nhrtf-paths = " + augs::get_executable_directory() + "\\hrtf";
		alsoft_ini_file += typesafe_sprintf("\nsources = %x", max_number_of_sound_sources);

		auto where_openal_expects_alsoft_ini = GetProcPath();
		alstr_append_cstr(&where_openal_expects_alsoft_ini, "\\alsoft.ini");

		const auto alsoft_ini_path = std::string(alstr_get_cstr(where_openal_expects_alsoft_ini));

		augs::create_text_file(alsoft_ini_path, alsoft_ini_file);
	}

	audio_manager::audio_manager(const std::string output_device_name) {
#if BUILD_OPENAL
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

		const auto all_audio_devices = list_audio_devices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));

		LOG(all_audio_devices);

		augs::create_text_file(
			std::string("generated/logs/audio_devices.txt"), 
			all_audio_devices
		);

		ALint hrtf_status;
		alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtf_status);

		LOG("Default device: %x", alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));
		LOG("HRTF status: %x", hrtf_status);
#endif
	}

	bool audio_manager::make_current() {
#if BUILD_OPENAL
		return (alcMakeContextCurrent(context)) == ALC_TRUE;
#else
		return true;
#endif
	}

	audio_manager::~audio_manager() {
#if BUILD_OPENAL
		AL_CHECK(alcMakeContextCurrent(nullptr));
		alcDestroyContext(context);
		alcCloseDevice(device);
		LOG("Destroyed audio manager");
		context = nullptr;
		device = nullptr;
#endif
	}
}