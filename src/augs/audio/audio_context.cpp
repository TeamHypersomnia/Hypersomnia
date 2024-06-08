#include "audio_context.h"

#include "augs/audio/OpenAL_error.h"

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#if PLATFORM_WEB

#define AL_SOURCE_DISTANCE_MODEL                 0x200

#define ALC_HRTF_SOFT                            0x1992
#define ALC_HRTF_STATUS_SOFT                     0x1993
#define ALC_HRTF_DISABLED_SOFT                   0x0000
#define ALC_HRTF_ENABLED_SOFT                    0x0001
#define ALC_HRTF_DENIED_SOFT                     0x0002
#define ALC_HRTF_REQUIRED_SOFT                   0x0003
#define ALC_HRTF_HEADPHONES_DETECTED_SOFT        0x0004
#define ALC_HRTF_UNSUPPORTED_FORMAT_SOFT         0x0005

typedef ALCboolean (*ALC_RESET_DEVICE_SOFT)(ALCdevice *, const ALCint *attribs);

ALC_RESET_DEVICE_SOFT alcResetDeviceSOFT;

#else
#include <AL/alext.h>
#include <AL/efx.h>
#endif
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
		const auto custom_hrtf_path = (std::filesystem::current_path() / "detail" / "hrtf").string();
		LOG_NVPS(custom_hrtf_path);

		setenv("ALSOFT_LOCAL_PATH", custom_hrtf_path.c_str(), 0);

		device = alcOpenDevice(device_name.size() > 0 ? device_name.c_str() : nullptr);
		
		AL_CHECK_DEVICE(device);

		if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
			LOG("Warning! ALC_EXT_EFX extension is not present.");
		}

		ALCint sample_rate = 0;
		alcGetIntegerv(device, ALC_FREQUENCY, 1, &sample_rate);
		LOG("Device sample rate: %x", sample_rate);
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

	std::string audio_device::get_output_mode() const {
#if BUILD_OPENAL && !PLATFORM_WEB
		ALint mode;
		alcGetIntegerv(device, ALC_OUTPUT_MODE_SOFT, 1, &mode);

		switch (mode) {
			case ALC_ANY_SOFT: return "ALC_ANY_SOFT";
			case ALC_STEREO_BASIC_SOFT: return "ALC_STEREO_BASIC_SOFT";
			case ALC_STEREO_UHJ_SOFT: return "ALC_STEREO_UHJ_SOFT";
			case ALC_STEREO_HRTF_SOFT: return "ALC_STEREO_HRTF_SOFT";
			case ALC_SURROUND_5_1_SOFT: return "ALC_SURROUND_5_1_SOFT";
			case ALC_SURROUND_6_1_SOFT: return "ALC_SURROUND_6_1_SOFT";
			case ALC_SURROUND_7_1_SOFT: return "ALC_SURROUND_7_1_SOFT";
			default: return "UNKNOWN";
		}
#else
		return "UNKNOWN";
#endif
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

#if BUILD_OPENAL
	static std::array<ALCint, 9> make_attrs(const audio_settings& settings) {
#if PLATFORM_WEB
		return {
			ALC_HRTF_SOFT, settings.output_mode == audio_output_mode::STEREO_HRTF, /* request HRTF */
			ALC_MONO_SOURCES, static_cast<ALCint>(settings.max_number_of_sound_sources),
			0
		};
#else
		auto output_mode_to_al_mode = [](const audio_output_mode mode) {
			using O = audio_output_mode;
			switch (mode) {
				case O::AUTO: return ALC_ANY_SOFT;
				case O::STEREO_BASIC: return ALC_STEREO_BASIC_SOFT;
				case O::STEREO_UHJ: return ALC_STEREO_UHJ_SOFT;
				case O::STEREO_HRTF: return ALC_STEREO_HRTF_SOFT;
				case O::SURROUND_5_1: return ALC_SURROUND_5_1_SOFT;
				case O::SURROUND_6_1: return ALC_SURROUND_6_1_SOFT;
				case O::SURROUND_7_1: return ALC_SURROUND_7_1_SOFT;
				default: return ALC_ANY_SOFT;
			}
		};

		return {
			ALC_MONO_SOURCES, static_cast<ALCint>(settings.max_number_of_sound_sources),
			ALC_OUTPUT_MODE_SOFT, output_mode_to_al_mode(settings.output_mode),
			ALC_OUTPUT_LIMITER_SOFT, AL_TRUE,
			0
		};
#endif
	}
#endif

	void audio_device::reset_device(audio_settings settings) {
#if BUILD_OPENAL
		auto attrs = make_attrs(settings);

		if ((bool)alcResetDeviceSOFT) {
			alcResetDeviceSOFT(device, &attrs[0]);
		}

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
		auto attrs = make_attrs(settings);
		context = alcCreateContext(device, &attrs[0]);

		if (!context || !set_as_current()) {
			if (context) {
				AL_CHECK(alcDestroyContext(context));
			}

			throw audio_error(
				"Failed to set an OpenAL context on device: %x", 
				settings.output_device_name
			);
		}

#if PLATFORM_WEB
		alcResetDeviceSOFT = reinterpret_cast<ALC_RESET_DEVICE_SOFT>(alcGetProcAddress(device, "alcResetDeviceSOFT"));
#endif

		AL_CHECK(alEnable(AL_SOURCE_DISTANCE_MODEL));
		LOG_NVPS(alIsEnabled(AL_SOURCE_DISTANCE_MODEL));

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

		if (// force // don't have to force it since we're setting it on alcCreateContext
			changed(settings.output_mode)
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
