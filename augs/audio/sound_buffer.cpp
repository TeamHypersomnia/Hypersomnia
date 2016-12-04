#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>

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

	sound_buffer::operator ALuint() const {
		return get_id();
	}
}
