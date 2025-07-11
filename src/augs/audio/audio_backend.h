#pragma once
#include <cstddef>
#include "augs/audio/sound_source.h"
#include "augs/audio/sound_sizes.h"

using ALuint = unsigned int;

namespace augs {
	struct audio_command;

	class audio_backend {
		sound_source flash_noise_source;

		std::array<sound_source, SOUNDS_SOURCES_IN_POOL> source_pool;

		ALuint lowpass_filter_id;

	public:
		unsigned get_max_texture_size() const;

		audio_backend();
		~audio_backend();

		audio_backend(audio_backend&&) = delete;
		audio_backend& operator=(audio_backend&&) = delete;

		audio_backend(const audio_backend&) = delete;
		audio_backend& operator=(const audio_backend&) = delete;

		void perform(
			const audio_command*, 
			std::size_t n
		);

		template <class F>
		void stop_sources_if(F pred) {
			auto maybe_stop = [&](auto& src) {
				if (pred(src.get_attached_buffer_id())) {
					src.stop();
					src.unbind_buffer();
				}
			};

			maybe_stop(flash_noise_source);

			for (auto& s : source_pool) {
				maybe_stop(s);
			}
		}
	};
}
