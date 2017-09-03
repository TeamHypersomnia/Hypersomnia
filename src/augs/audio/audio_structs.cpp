#include "audio_structs.h"

#include "augs/audio/OpenAL_error.h"

#if BUILD_OPENAL
extern "C" {
	#include <compat.h>
}

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/alext.h>
#endif

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/audio/audio_settings.h"
#include "augs/templates/corresponding_field.h"

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
	void generate_alsoft_ini(
		const unsigned max_number_of_sound_sources
	) {
#if BUILD_OPENAL
		std::string alsoft_ini_file;
		alsoft_ini_file += "# Do not modify.";
		alsoft_ini_file += "\n# Hypersomnia generates this file every launch to speak with OpenAL.";
		alsoft_ini_file += "\n# Modification will have no effect.";
		alsoft_ini_file += "\nhrtf-paths = " + augs::get_executable_directory().string() + "\\hrtf";
		alsoft_ini_file += typesafe_sprintf("\nsources = %x", max_number_of_sound_sources);

		auto where_openal_expects_alsoft_ini = GetProcPath();
		alstr_append_cstr(&where_openal_expects_alsoft_ini, "\\alsoft.ini");

		const auto alsoft_ini_path = augs::path_type(alstr_get_cstr(where_openal_expects_alsoft_ini));

		augs::create_text_file(alsoft_ini_path, alsoft_ini_file);
#endif
	}

	void log_all_audio_devices(const path_type& output_path) {
#if BUILD_OPENAL
		const auto all_audio_devices = list_audio_devices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));

		LOG(all_audio_devices);
		LOG("Default device: %x", alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));

		augs::create_text_file(
			output_path,
			all_audio_devices
		);
#endif
	}

	audio_device::audio_device(const std::string& device_name) {
#if BUILD_OPENAL
		device = alcOpenDevice(device_name.size() > 0 ? device_name.c_str() : nullptr);
		
		AL_CHECK_DEVICE(device);

		if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
			LOG("Warning! ALC_EXT_EFX extension is not present.");
		}
#endif
	}

	audio_device::~audio_device() {
		destroy();
	}

	audio_device::audio_device(audio_device&& b) noexcept
		: device(b.device)
	{
		b.device = nullptr;
	}

	audio_device& audio_device::operator=(audio_device&& b) noexcept {
		destroy();

		device = b.device;

		b.device = nullptr;
		return *this;
	}

	void audio_device::destroy() {
#if BUILD_OPENAL
		if (device != nullptr) {
			alcCloseDevice(device);
			LOG("Destroyed OpenAL device: %x", device);
			device = nullptr;
		}
#endif
	}

	void audio_device::log_hrtf_status() const {
#if BUILD_OPENAL
		ALint hrtf_status;
		alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtf_status);
		AL_CHECK_DEVICE(device);

		std::string status;

		switch (hrtf_status) {
		case ALC_HRTF_DISABLED_SOFT: status = "ALC_HRTF_DISABLED_SOFT"; break;
		case ALC_HRTF_ENABLED_SOFT: status = "ALC_HRTF_ENABLED_SOFT"; break;
		case ALC_HRTF_DENIED_SOFT: status = "ALC_HRTF_DENIED_SOFT"; break;
		case ALC_HRTF_REQUIRED_SOFT: status = "ALC_HRTF_REQUIRED_SOFT"; break;
		case ALC_HRTF_HEADPHONES_DETECTED_SOFT: status = "ALC_HRTF_HEADPHONES_DETECTED_SOFT"; break;
		case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT: status = "ALC_HRTF_UNSUPPORTED_FORMAT_SOFT"; break;
		default: status = "Unknown"; break;
		}

		LOG("HRTF status: %x", status);
#endif
	}

	void audio_device::set_hrtf_enabled(const bool enabled) {
#if BUILD_OPENAL
		ALCint attrs[] = {
			ALC_HRTF_SOFT, enabled, /* request HRTF */
			0 /* end of list */
		};

		alcResetDeviceSOFT(device, attrs);
		AL_CHECK_DEVICE(device);
		log_hrtf_status();
#endif
	}

	audio_context::audio_context(const audio_settings& settings) 
		: device(settings.output_device_name) 
	{
#if BUILD_OPENAL
		context = alcCreateContext(device, nullptr);

		if (!context || !set_as_current()) {
			if (context) {
				AL_CHECK(alcDestroyContext(context));
			}

			throw audio_error(
				"Failed to set an OpenAL context on device: %x", 
				settings.output_device_name
			);
		}

		AL_CHECK(alSpeedOfSound(100.f));
		AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
		AL_CHECK(alListenerf(AL_METERS_PER_UNIT, 1.3f));

		device.log_hrtf_status();
		apply(settings, true);
#endif
	}

	audio_context::~audio_context() {
		destroy();
	}

	audio_context::audio_context(audio_context&& b) noexcept :
		device(std::move(b.device)),
		context(b.context)
	{
		b.context = nullptr;
	}

	audio_context& audio_context::operator=(audio_context&& b) noexcept {
		destroy();

		device = std::move(b.device);
		context = b.context;

		b.context = nullptr;

		return *this;
	}

	void audio_context::destroy() {
#if BUILD_OPENAL
		if (context) {
			alcMakeContextCurrent(nullptr);
			alcDestroyContext(context);
			LOG("Destroyed OpenAL context: %x", context);
			context = nullptr;
		}
#endif
	}

	bool audio_context::set_as_current() {
#if BUILD_OPENAL
		auto result = alcMakeContextCurrent(context);
		AL_CHECK(1);
		return result == ALC_TRUE;
#else
		return true;
#endif
	}

	void audio_context::apply(const audio_settings& settings, const bool force) {
		auto changed = [&](auto& field) {
			return !(field == augs::get_corresponding_field(field, settings, current_settings));
		};

		if (force || changed(settings.enable_hrtf)) {
			device.set_hrtf_enabled(settings.enable_hrtf);
		}

		current_settings = settings;
	}
}