#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

#include "augs/log.h"

namespace augs {
	sound_buffer::sound_buffer() {
		alGenBuffers(1, &id);
	}

	sound_buffer::~sound_buffer() {
		alDeleteBuffers(1, &id);
	}

	ALuint sound_buffer::get_id() const {
		return id;
	}

	void sound_buffer::set_data(const data_type new_data) {
		data = new_data;
		alBufferData(id, new_data.get_format(), new_data.samples.data(), new_data.samples.size(), new_data.frequency);
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

		LOG("Sound: %x\nSample rate: %x\nChannels: %x\nLength in seconds: %x", filename, info.samplerate, info.channels, get_length_in_seconds());

		sf_close(file);
	}

	double sound_buffer::get_length_in_seconds() const {
		return static_cast<double>(data.samples.size()) / (data.frequency * data.channels);
	}
}
