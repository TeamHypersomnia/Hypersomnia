#include "audio_context.h"

#include "augs/audio/OpenAL_error.h"

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/alext.h>
#endif

#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/audio/audio_settings.h"
#include "augs/templates/corresponding_field.h"

#if PLATFORM_WINDOWS
int setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#endif

#if BUILD_OPENAL
static std::string list_audio_devices(const ALCchar * const devices) {
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	std::string devices_list;

	devices_list += "Devices list:\n";

	while (device && *device != '\0' && next && *next != '\0') {
		devices_list += typesafe_sprintf("%x\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}

	return devices_list;
}
#endif

namespace augs {
	void log_all_audio_devices(const path_type& output_path) {
#if BUILD_OPENAL
		const auto all_audio_devices = list_audio_devices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));

		LOG(all_audio_devices);
		LOG("Default device: %x", alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));

		augs::save_as_text(
			output_path,
			all_audio_devices
		);
#else
		(void)output_path;
#endif
	}

	audio_device::audio_device(const std::string& device_name) {
#if BUILD_OPENAL
		const auto hrtf_path = (std::filesystem::current_path() / "content" / "hrtf").string();
		LOG_NVPS(hrtf_path);

		setenv("ALSOFT_LOCAL_PATH", hrtf_path.c_str(), 0);

		device = alcOpenDevice(device_name.size() > 0 ? device_name.c_str() : nullptr);
		
		AL_CHECK_DEVICE(device);

		if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
			LOG("Warning! ALC_EXT_EFX extension is not present.");
		}
#else
		(void)device_name;
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
		const auto stat = get_hrtf_status();
		LOG(stat.message);
	}

	audio_device::hrtf_stat audio_device::get_hrtf_status() const {
#if BUILD_OPENAL
		ALint hrtf_status;
		alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtf_status);
		AL_CHECK_DEVICE(device);

		std::string status;
		bool succ = false;

		switch (hrtf_status) {
		case ALC_HRTF_DISABLED_SOFT: status = "ALC_HRTF_DISABLED_SOFT"; break;
		case ALC_HRTF_ENABLED_SOFT: succ = true; status = "ALC_HRTF_ENABLED_SOFT"; break;
		case ALC_HRTF_DENIED_SOFT: status = "ALC_HRTF_DENIED_SOFT"; break;
		case ALC_HRTF_REQUIRED_SOFT: succ = true; status = "ALC_HRTF_REQUIRED_SOFT"; break;
		case ALC_HRTF_HEADPHONES_DETECTED_SOFT: succ = true; status = "ALC_HRTF_HEADPHONES_DETECTED_SOFT"; break;
		case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT: status = "ALC_HRTF_UNSUPPORTED_FORMAT_SOFT"; break;
		default: status = "Unknown"; break;
		}

		return { succ, status };
#else
		return { false, "ALC_HRTF_DISABLED_SOFT" };
#endif
	}

	void audio_device::reset_device(audio_settings settings) {
#if BUILD_OPENAL
		ALCint attrs[] = {
			ALC_HRTF_SOFT, settings.enable_hrtf, /* request HRTF */
			ALC_MONO_SOURCES, static_cast<ALCint>(settings.max_number_of_sound_sources),
			ALC_OUTPUT_LIMITER_SOFT, AL_TRUE,
		   	0	/* end of list */
		};

		alcResetDeviceSOFT(device, attrs);
		AL_CHECK_DEVICE(device);
		log_hrtf_status();
#else
		(void)settings;
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

		AL_CHECK(alEnable(AL_SOURCE_DISTANCE_MODEL));

		device.log_hrtf_status();

		apply(settings, true);

		{
			ALint num_stereo_sources;
			ALint num_mono_sources;

			alcGetIntegerv(device, ALC_STEREO_SOURCES, 1, &num_stereo_sources);
			alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &num_mono_sources);

			LOG_NVPS(num_stereo_sources, num_mono_sources);
		}
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
		AL_CHECK(result);
		return result == ALC_TRUE;
#else
		return true;
#endif
	}

	void audio_context::speed_of_sound(const float meters_per_sec) {
		(void)meters_per_sec;
		AL_CHECK(alSpeedOfSound(meters_per_sec));
	}

	void audio_context::apply(const audio_settings& settings, const bool force) {
		auto changed = [&](auto& field) {
			return !(field == augs::get_corresponding_field(field, settings, current_settings));
		};

		if (force 
			|| changed(settings.enable_hrtf)
			|| changed(settings.max_number_of_sound_sources)
		) {
			device.reset_device(settings);
		}

		if (force || changed(settings.sound_meters_per_second)) {
			speed_of_sound(settings.sound_meters_per_second);
		}

		current_settings = settings;
	}
}
