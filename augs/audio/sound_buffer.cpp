#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.h>

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
		alBufferData(id, new_data.format, new_data.data.data(), new_data.data.size(), new_data.freq);
	}

	sound_buffer::data_type sound_buffer::get_data() const {
		return data;
	}

	sound_buffer::operator ALuint() const {
		return get_id();
	}

	void sound_buffer::from_file(const std::string filename) {
		SF_INFO info;
		SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &info);
	}
}
