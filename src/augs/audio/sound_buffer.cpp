#include <array>

#include "generated/setting_build_openal.h"

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "augs/ensure.h"
#include "augs/al_log.h"

#include "augs/filesystem/file.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_samples_from_file.h"

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
		ensure(!bad_format);
		return AL_FORMAT_MONO8;
#else
		return 0xdeadbeef;
#endif
	}

	single_sound_buffer::~single_sound_buffer() {
		if (initialized) {
#if TRACE_CONSTRUCTORS_DESTRUCTORS
			--g_num_buffers;
			LOG("alDeleteBuffers: %x (now %x buffers)", id, g_num_buffers);
#endif
			AL_CHECK(alDeleteBuffers(1, &id));
			initialized = false;
		}
	}

	single_sound_buffer& single_sound_buffer::operator=(single_sound_buffer&& b) {
		std::swap(initialized, b.initialized);
		std::swap(computed_length_in_seconds, b.computed_length_in_seconds);
		std::swap(id, b.id);
		return *this;
	}
	
	single_sound_buffer::single_sound_buffer(single_sound_buffer&& b) {
		*this = std::move(b);
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
		computed_length_in_seconds = new_data.compute_length_in_seconds();

#if LOG_AUDIO_BUFFERS
		LOG("Passed format: %x\nPassed frequency: %x\nPassed bytesize: %x", passed_format, passed_frequency, passed_bytesize);
#endif

		AL_CHECK(alBufferData(id, passed_format, new_data.samples.data(), passed_bytesize, passed_frequency));
	}

	//sound_data single_sound_buffer::get_data() const {
	//	return data;
	//}

	bool single_sound_buffer::is_set() const {
		return initialized;
	}

	double single_sound_buffer::get_length_in_seconds() const {
		return computed_length_in_seconds;
	}

	single_sound_buffer& sound_buffer::variation::request_original() {
		if (original_channels == 1) {
			return mono;
		}
		else if (original_channels == 2) {
			return stereo;
		}

		const bool bad_format = true;
		ensure(!bad_format);

		return mono;
	}

	single_sound_buffer& sound_buffer::variation::request_mono() {
		ensure(mono.is_set());
		return mono;
	}

	single_sound_buffer& sound_buffer::variation::request_stereo() {
		if (!stereo.is_set()) {
			return request_mono();
		}

		return stereo;
	}

	const single_sound_buffer& sound_buffer::variation::request_original() const {
		if (original_channels == 1) {
			return request_mono();
		}
		else if (original_channels == 2) {
			return request_stereo();
		}

		const bool bad_format = true;
		ensure(!bad_format);

		return mono;
	}

	const single_sound_buffer& sound_buffer::variation::request_mono() const {
		ensure(mono.is_set());
		return mono;
	}

	const single_sound_buffer& sound_buffer::variation::request_stereo() const {
		if (!stereo.is_set()) {
			return request_mono();
		}

		return stereo;
	}

	void sound_buffer::variation::set_data(const sound_data& data, const bool generate_mono) {
		original_channels = data.channels;

		if (data.channels == 1) {
			mono.set_data(data);
		}
		else if (data.channels == 2) {
			stereo.set_data(data);

			if (generate_mono) {
				mono.set_data(mix_stereo_to_mono(data));
			}

		}
		else {
			const bool bad_format = true;
			ensure(!bad_format);
		}
	}

	ALuint sound_buffer::get_id() const {
		return variations[0].request_original();
	}

	sound_buffer::operator ALuint() const {
		return get_id();
	}
	
	sound_buffer_logical_meta sound_buffer::get_logical_meta(const assets_manager& manager) const {
		sound_buffer_logical_meta output;
		output.num_of_variations = variations.size();

		const auto len = [](const variation& v) {
			return v.request_original().get_length_in_seconds();
		};

		output.max_duration_in_seconds = static_cast<float>(len(*std::max_element(
			variations.begin(),
			variations.end(),
			[len](const variation& a, const variation& b) {
				return len(a) < len(b);
			}
		)));

		return output;
	}

	void sound_buffer::from_file(const std::string& path, const bool generate_mono) {
		for (size_t i = 1;;++i) {
			const auto target_path = typesafe_sprintf(path, i);

			const bool no_change_in_path = target_path == path;

			if (!augs::file_exists(target_path) || (i > 1 && no_change_in_path)) {
				break;
			}

			variation new_variation;
			new_variation.set_data(get_sound_samples_from_file(target_path), generate_mono);
			variations.emplace_back(std::move(new_variation));
		}

		ensure(variations.size() > 0);
	}

	size_t sound_buffer::get_num_variations() const {
		return variations.size();
	}

	sound_buffer::variation& sound_buffer::get_variation(const size_t i) {
		return variations.at(i);
	}

	const sound_buffer::variation& sound_buffer::get_variation(const size_t i) const {
		return variations.at(i);
	}
}
