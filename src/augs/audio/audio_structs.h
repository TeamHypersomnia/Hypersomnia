#pragma once
#include <string>

/** Opaque device handle */
typedef struct ALCdevice_struct ALCdevice;
/** Opaque context handle */
typedef struct ALCcontext_struct ALCcontext;

namespace augs {
	void generate_alsoft_ini(
		const bool hrtf_enabled,
		const unsigned max_number_of_sound_sources
	);
	
	void log_all_audio_devices(const std::string& output_path);
	
	class audio_device {
		ALCdevice* device = nullptr;

		audio_device(const audio_device&) = delete;
		audio_device(audio_device&&) = delete;
		audio_device& operator=(const audio_device&) = delete;
		audio_device& operator=(audio_device&&) = delete;

	public:
		auto* get() {
			return device;
		}

		audio_device(const std::string& device_name = "");
		~audio_device();
	};

	class audio_context {
		ALCcontext* context = nullptr;
		
		audio_context(const audio_context&) = delete;
		audio_context(audio_context&&) = delete;
		audio_context& operator=(const audio_context&) = delete;
		audio_context& operator=(audio_context&&) = delete;

	public:
		audio_context(audio_device& device);
		~audio_context();

		bool make_current();
	};
}