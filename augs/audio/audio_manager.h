#pragma once

/** Opaque device handle */
typedef struct ALCdevice_struct ALCdevice;
/** Opaque context handle */
typedef struct ALCcontext_struct ALCcontext;

namespace augs {
	class audio_manager {
		ALCdevice* device = nullptr;
		ALCcontext* context = nullptr;
		
		audio_manager(const audio_manager&) = delete;
		audio_manager(audio_manager&&) = delete;
		audio_manager& operator=(const audio_manager&) = delete;
		audio_manager& operator=(audio_manager&&) = delete;

	public:
		static void generate_alsoft_ini(const bool hrtf_enabled);

		audio_manager();
		~audio_manager();

		bool make_current();
	};
}