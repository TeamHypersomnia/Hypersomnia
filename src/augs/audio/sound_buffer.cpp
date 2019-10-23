#include "augs/log.h"

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "augs/ensure.h"
#include "augs/audio/OpenAL_error.h"

#include "augs/filesystem/file.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"

#include "augs/string/string_templates.h"

#define TRACE_CONSTRUCTORS_DESTRUCTORS 0

#if TRACE_CONSTRUCTORS_DESTRUCTORS
int g_num_buffers = 0;
#endif

namespace augs {
	ALenum get_openal_format_of(const sound_data& d) {
#if BUILD_OPENAL
		if (d.channels == 1) {
			return AL_FORMAT_MONO16;
		}
		else if (d.channels == 2) {
			return AL_FORMAT_STEREO16;
		}

		const bool bad_format = true;
		(void)bad_format;

		ensure(!bad_format);
		return AL_FORMAT_MONO8;
#else
		(void)d;
		return 0xdeadbeef;
#endif
	}

	single_sound_buffer::single_sound_buffer(const sound_data& data, const sound_buffer_loading_settings) {
		set_data(data);
	}

	single_sound_buffer::single_sound_buffer(const sound_data& data) : single_sound_buffer(data, sound_buffer_loading_settings()) {}

	single_sound_buffer::~single_sound_buffer() {
		destroy();
	}

	single_sound_buffer::single_sound_buffer(single_sound_buffer&& b) : 
		meta(std::move(b.meta)),
		id(b.id),
		initialized(b.initialized)
	{
		b.initialized = false;
		b.meta = {};
	}

	single_sound_buffer& single_sound_buffer::operator=(single_sound_buffer&& b) {
		destroy();

		meta = std::move(b.meta);
		id = b.id;
		initialized = b.initialized;

		b.initialized = false;
		b.meta = {};

		return *this;
	}

	void single_sound_buffer::destroy() {
		if (initialized) {
#if TRACE_CONSTRUCTORS_DESTRUCTORS
			--g_num_buffers;
			LOG("alDeleteBuffers: %x (now %x buffers)", id, g_num_buffers);
#endif
			AL_CHECK(alDeleteBuffers(1, &id));
			initialized = false;
		}
	}

	ALuint single_sound_buffer::get_id() const {
		return id;
	}

	single_sound_buffer::operator ALuint() const {
		return get_id();
	}

	void single_sound_buffer::set_data(const sound_data& new_data) {
		if (!initialized) {
			AL_CHECK(alGenBuffers(1, &id));

#if TRACE_CONSTRUCTORS_DESTRUCTORS
			++g_num_buffers;
			LOG("alGenBuffers: %x (now %x buffers)", id, g_num_buffers);
#endif
			initialized = true;
		}

		if (new_data.samples.empty()) {
			LOG("WARNING! No samples were sent to a sound buffer.");
			return;
		}

		const auto passed_format = get_openal_format_of(new_data);
		const auto passed_frequency = new_data.frequency;
		const auto passed_bytesize = new_data.samples.size() * sizeof(sound_sample_type);
		meta.computed_length_in_seconds = new_data.compute_length_in_seconds();

#if LOG_AUDIO_BUFFERS
		LOG("Passed format: %x\nPassed frequency: %x\nPassed bytesize: %x", passed_format, passed_frequency, passed_bytesize);
#endif

		(void)passed_bytesize;
		(void)passed_frequency;
		(void)passed_format;
		AL_CHECK(alBufferData(id, passed_format, new_data.samples.data(), static_cast<ALsizei>(passed_bytesize), static_cast<ALsizei>(passed_frequency)));
	}

	double single_sound_buffer::get_length_in_seconds() const {
		return meta.computed_length_in_seconds;
	}

	sound_buffer::sound_buffer(const sound_buffer_loading_input input) {
		from_file(input);
	}

	void sound_buffer::from_file(const sound_buffer_loading_input input) {
		const auto& path = input.source_sound;
		variations.emplace_back(path, input.settings);

		const auto ext = augs::path_type(path).extension();
		const auto without_ext = augs::path_type(path).replace_extension("").string();

		if (ends_with(without_ext, "_1")) {
			const auto without_num = without_ext.substr(0, without_ext.size() - 2);

			for (size_t i = 2;; ++i) {
				const auto next_path = augs::path_type(typesafe_sprintf("%x_%x%x", without_num, i, ext));

				try {
					variations.emplace_back(next_path, input.settings);
				}
				catch (...) {
					break;
				}
			}
		}
	}

	const single_sound_buffer& sound_buffer::get_buffer(const std::size_t variation_index) const {
		const auto chosen_variation = variation_index % variations.size();
		return variations[chosen_variation];
	}
}
