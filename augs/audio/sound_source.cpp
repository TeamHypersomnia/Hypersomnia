#include "sound_source.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <xutility>

namespace augs {
	sound_source::sound_source() {
		alGenSources(1, &id);
		initialized = true;
	}

	sound_source::~sound_source() {
		if (initialized) {
			alDeleteSources(1, &id);
			initialized = false;
		}
	}

	sound_source::sound_source(sound_source&& b) {
		*this = std::move(b);
	}

	sound_source& sound_source::operator=(sound_source&& b) {
		initialized = b.initialized;
		id = b.id;
		b.initialized = false;
		return *this;
	}

	ALuint sound_source::get_id() const {
		return id;
	}

	sound_source::operator ALuint() const {
		return get_id();
	}
}
