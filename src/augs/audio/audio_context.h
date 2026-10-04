#pragma once
#include <string>
#include <vector>
#include <optional>
#include "augs/filesystem/path.h"
#include "augs/templates/exception_templates.h"
#include "augs/audio/audio_settings.h"

#if PLATFORM_WEB
typedef struct ALCcontext_struct ALCcontext;
typedef struct ALCdevice_struct ALCdevice;
#else
/** Opaque device handle */
typedef struct ALCdevice ALCdevice;
/** Opaque context handle */
typedef struct ALCcontext ALCcontext;
#endif

namespace augs {
	struct audio_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	void log_all_audio_devices(const path_type& output_path);

	struct audio_paths {
		path_type official_hrtfs_dir;
		path_type user_hrtfs_dir;
		path_type generated_openal_config;
	};

	struct hrtf_presets {
		std::vector<std::string> official;
		std::vector<std::string> user;
	};

	hrtf_presets find_hrtf_presets(const audio_paths&);
	
	class audio_device {
		friend class audio_context;

		ALCdevice* device = nullptr;

		void destroy();

		audio_paths paths;
		unsigned period_size_at_launch = 0;

		audio_device(const audio_settings&, const audio_paths&);
		~audio_device();

		audio_device(audio_device&&) noexcept;
		audio_device& operator=(audio_device&&) noexcept;

		audio_device(const audio_device&) = delete;
		audio_device& operator=(const audio_device&) = delete;

	public:
		struct hrtf_stat {
			bool success = false;
			std::string message;
		};

		void reset_device(audio_settings);
		hrtf_stat get_hrtf_status() const;
		std::string get_output_mode() const;
		std::string get_hrtf_name() const;
		std::optional<float> get_latency_ms() const;
		int find_hrtf_id(const audio_settings&) const;

		const auto& get_paths() const {
			return paths;
		}

		/* OpenAL reads the period size only once per process. */
		auto get_period_size_at_launch() const {
			return period_size_at_launch;
		}
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
		audio_context(const audio_settings&, const audio_paths&);
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

		void apply(const audio_settings&, bool force = false);
	};
}