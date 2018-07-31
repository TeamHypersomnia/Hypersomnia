#pragma once
#include <string>
#include "augs/filesystem/path.h"
#include "augs/templates/exception_templates.h"
#include "augs/audio/audio_settings.h"

/** Opaque device handle */
typedef struct ALCdevice_struct ALCdevice;
/** Opaque context handle */
typedef struct ALCcontext_struct ALCcontext;

namespace augs {
	struct audio_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	void log_all_audio_devices(const path_type& output_path);
	
	class audio_device {
		friend class audio_context;

		ALCdevice* device = nullptr;

		void destroy();

		audio_device(const std::string& device_name);
		~audio_device();

		audio_device(audio_device&&) noexcept;
		audio_device& operator=(audio_device&&) noexcept;

		audio_device(const audio_device&) = delete;
		audio_device& operator=(const audio_device&) = delete;

	public:
		void reset_device(audio_settings);
		void log_hrtf_status() const;

		operator ALCdevice*() {
			return device;
		}

		operator const ALCdevice*() const {
			return device;
		}
	};
	
	/* We enforce just one context per every device to avoid unnecessary drama. */

	class audio_context {
		audio_device device;
		ALCcontext* context = nullptr;
		audio_settings current_settings;

		void destroy();
		bool set_as_current();
		void speed_of_sound(float);

	public:
		audio_context(const audio_settings& device_name);
		~audio_context();

		audio_context(audio_context&&) noexcept;
		audio_context& operator=(audio_context&&) noexcept;

		audio_context(const audio_context&) = delete;
		audio_context& operator=(const audio_context&) = delete;

		audio_device& get_device() {
			return device;
		}

		const audio_device& get_device() const {
			return device;
		}

		void apply(const audio_settings&, const bool force = false);
	};
}