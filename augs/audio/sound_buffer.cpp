#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

#include "augs/al_log.h"
#include "augs/ensure.h"

namespace augs {
	sound_buffer::sound_buffer() {
		AL_CHECK(alGenBuffers(1, &id));
	}

	sound_buffer::~sound_buffer() {
		AL_CHECK(alDeleteBuffers(1, &id));
	}

	ALuint sound_buffer::get_id() const {
		return id;
	}

	void sound_buffer::set_data(const data_type new_data) {
		data = new_data;
		const auto passed_format = new_data.get_format();
		const auto passed_frequency = new_data.frequency;
		const auto passed_bytesize = new_data.samples.size() * sizeof(int16_t);

		LOG("Passed format: %x\nPassed frequency: %x\nPassed bytesize: %x", passed_format, passed_frequency, passed_bytesize);

		AL_CHECK(alBufferData(id, passed_format, new_data.samples.data(), passed_bytesize, passed_frequency));
	}

	sound_buffer::data_type sound_buffer::get_data() const {
		return data;
	}

	sound_buffer::operator ALuint() const {
		return get_id();
	}

	int sound_buffer::data_type::get_format() const {
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

	void sound_buffer::from_file(const std::string filename) {
		SF_INFO info;
		std::memset(&info, 0, sizeof(info));

		SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &info);
		
		data_type new_data;

		new_data.channels = info.channels;
		new_data.frequency = info.samplerate;

		std::array<int16_t, 4096> read_buf;
		size_t read_size = 0;

		while ((read_size = sf_read_short(file, read_buf.data(), read_buf.size())) != 0) {
			new_data.samples.insert(new_data.samples.end(), read_buf.begin(), read_buf.begin() + read_size);
		}

		set_data(new_data);

		LOG("Sound: %x\nSample rate: %x\nChannels: %x\nFormat: %x\nLength in seconds: %x", 
			filename, 
			info.samplerate, 
			info.channels, 
			info.format, 
			get_length_in_seconds());

		sf_close(file);
	}

	double sound_buffer::get_length_in_seconds() const {
		return static_cast<double>(data.samples.size()) / (data.frequency * data.channels);
	}
}
