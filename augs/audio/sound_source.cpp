#include "sound_source.h"

#include <AL/al.h>
#include <AL/alc.h>

namespace augs {
	sound_source::sound_source() {
		alGenSources(1, &id);
	}

	sound_source::~sound_source() {
		alDeleteSources(1, &id);
	}

	ALuint sound_source::get_id() const {
		return id;
	}

	sound_source::operator ALuint() const {
		return get_id();
	}
}
