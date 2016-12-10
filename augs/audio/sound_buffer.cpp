#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

#include "augs/al_log.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"

namespace augs {
	single_sound_buffer::~single_sound_buffer() {
		if (initialized) {
			AL_CHECK(alDeleteBuffers(1, &id));
			initialized = false;
		}
	}

	single_sound_buffer& single_sound_buffer::operator=(single_sound_buffer&& b) {
		initialized = b.initialized;
		computed_length_in_seconds = b.computed_length_in_seconds;
		id = b.id;
		b.initialized = false;
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

	void single_sound_buffer::set_data(const data_type& new_data) {
		if (!initialized) {
			AL_CHECK(alGenBuffers(1, &id));
			initialized = true;
		}

		const auto passed_format = new_data.get_format();
		const auto passed_frequency = new_data.frequency;
		const auto passed_bytesize = new_data.samples.size() * sizeof(int16_t);
		computed_length_in_seconds = new_data.compute_length_in_seconds();

		LOG("Passed format: %x\nPassed frequency: %x\nPassed bytesize: %x", passed_format, passed_frequency, passed_bytesize);

		AL_CHECK(alBufferData(id, passed_format, new_data.samples.data(), passed_bytesize, passed_frequency));
	}

	double single_sound_buffer::data_type::compute_length_in_seconds() const {
		return static_cast<double>(samples.size()) / (frequency * channels);
	}

	int single_sound_buffer::data_type::get_format() const {
		if (channels == 1) {
			return AL_FORMAT_MONO16;
		}
		else if (channels == 2) {
			return AL_FORMAT_STEREO16;
		}

		const bool bad_format = true;
		ensure(!bad_format);
		return AL_FORMAT_MONO8;
	}

	//single_sound_buffer::data_type single_sound_buffer::get_data() const {
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
		ensure(stereo.is_set());
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
		ensure(stereo.is_set());
		return stereo;
	}

	void sound_buffer::variation::set_data(const single_sound_buffer::data_type& data, const bool generate_mono) {
		original_channels = data.channels;

		if (data.channels == 1) {
			mono.set_data(data);
		}
		else if (data.channels == 2) {
			stereo.set_data(data);

			if (generate_mono) {
				single_sound_buffer::data_type mono_data;
				mono_data.channels = 1;
				mono_data.frequency = data.frequency;
				mono_data.samples = mix_stereo_to_mono(data.samples);

				mono.set_data(mono_data);
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
	
	std::vector<int16_t> sound_buffer::mix_stereo_to_mono(const std::vector<int16_t>& samples) {
		ensure(samples.size() % 2 == 0);
		
		std::vector<int16_t> output;
		output.resize(samples.size() / 2);

		for (size_t i = 0; i < samples.size(); i += 2) {
			output[i/2] = (static_cast<int>(samples[i]) + samples[i+1]) / 2;
		}

		return std::move(output);
	}

	single_sound_buffer::data_type sound_buffer::get_data_from_file(const std::string filename) {
		SF_INFO info;
		std::memset(&info, 0, sizeof(info));

		SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &info);

		single_sound_buffer::data_type new_data;

		new_data.channels = info.channels;
		new_data.frequency = info.samplerate;

		std::array<int16_t, 4096> read_buf;
		size_t read_size = 0;

		while ((read_size = sf_read_short(file, read_buf.data(), read_buf.size())) != 0) {
			new_data.samples.insert(new_data.samples.end(), read_buf.begin(), read_buf.begin() + read_size);
		}

		sf_close(file);

		LOG("Sound: %x\nSample rate: %x\nChannels: %x\nFormat: %x\nLength in seconds: %x",
			filename,
			info.samplerate,
			info.channels,
			info.format,
			new_data.compute_length_in_seconds());

		return std::move(new_data);
	}

	void sound_buffer::from_file(const std::string filename, const bool generate_mono) {
		for (size_t i = 1;;++i) {
			const auto target_filename = typesafe_sprintf(filename, i);

			const bool no_change_in_filename = target_filename == filename;

			if (!augs::file_exists(target_filename) || (i > 1 && no_change_in_filename)) {
				break;
			}

			variation new_variation;
			new_variation.set_data(get_data_from_file(target_filename), generate_mono);
			variations.emplace_back(std::move(new_variation));
		}
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
