#include "sound_source.h"
#include "sound_buffer.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "augs/al_log.h"
#include "game/components/physics_component.h"

#define Y_IS_Z 1

namespace augs {
	sound_source::sound_source() {
		AL_CHECK(alGenSources(1, &id));

		AL_CHECK(alSourcef(id, AL_PITCH, 1));
		AL_CHECK(alSourcef(id, AL_GAIN, 1));
		AL_CHECK(alSourcei(id, AL_LOOPING, AL_FALSE));

		initialized = true;
	}

	sound_source::~sound_source() {
		if (initialized) {
			AL_CHECK(alDeleteSources(1, &id));
			initialized = false;
			attached_buffer = nullptr;
		}
	}

	sound_source::sound_source(sound_source&& b) {
		*this = std::move(b);
	}

	sound_source& sound_source::operator=(sound_source&& b) {
		initialized = b.initialized;
		id = b.id;
		attached_buffer = b.attached_buffer;
		b.initialized = false;
		return *this;
	}

	ALuint sound_source::get_id() const {
		return id;
	}

	sound_source::operator ALuint() const {
		return get_id();
	}

	void sound_source::play() const {
		AL_CHECK(alSourcePlay(id));
	}

	void sound_source::stop() const {
		AL_CHECK(alSourceStop(id));
	}
	
	void sound_source::set_looping(const bool loop) const {
		AL_CHECK(alSourcei(id, AL_LOOPING, loop));
	}

	void sound_source::set_pitch(const float pitch) const {
		AL_CHECK(alSourcef(id, AL_PITCH, pitch));
	}

	void sound_source::set_gain(const float gain) const {
		AL_CHECK(alSourcef(id, AL_GAIN, gain));
	}

	void sound_source::set_velocity(vec2 v) const {
		v *= PIXELS_TO_METERSf;
#if Y_IS_Z
		AL_CHECK(alSource3f(id, AL_VELOCITY, v.x, 0.f, v.y));
#else
		AL_CHECK(alSource3f(id, AL_VELOCITY, v.x, v.y, 0.f));
#endif
	}

	void sound_source::set_position(vec2 pos) const {
		pos *= PIXELS_TO_METERSf;

#if Y_IS_Z
		AL_CHECK(alSource3f(id, AL_POSITION, pos.x, 0.f, pos.y));
#else
		AL_CHECK(alSource3f(id, AL_POSITION, pos.x, pos.y, 0.f));
#endif
	}

	void sound_source::set_max_distance(const float distance) const {
		AL_CHECK(alSourcef(id, AL_MAX_DISTANCE, distance * PIXELS_TO_METERSf));
	}

	void sound_source::set_reference_distance(const float distance) const {
		AL_CHECK(alSourcef(id, AL_REFERENCE_DISTANCE, distance * PIXELS_TO_METERSf));
	}

	void sound_source::set_direct_channels(const bool flag) const {
		AL_CHECK(alSourcei(id, AL_DIRECT_CHANNELS_SOFT, flag));
	}

	void sound_source::bind_buffer(const single_sound_buffer& buf) {
		attached_buffer = &buf;
		AL_CHECK(alSourcei(id, AL_BUFFER, buf.get_id()));
	}

	void sound_source::unbind_buffer() {
		attached_buffer = nullptr;
		AL_CHECK(alSourcei(id, AL_BUFFER, NULL));
	}
	
	const single_sound_buffer* sound_source::get_bound_buffer() const {
		return attached_buffer;
	}

	void set_listener_position(vec2 pos) {
		pos *= PIXELS_TO_METERSf;

#if Y_IS_Z
		AL_CHECK(alListener3f(AL_POSITION, pos.x, 0.f, pos.y));
#else
		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, 0.f));
#endif
	}

	void set_listener_velocity(vec2 v) {
		v *= PIXELS_TO_METERSf;

#if Y_IS_Z
		AL_CHECK(alListener3f(AL_VELOCITY, v.x, 0.f, v.y));
#else
		AL_CHECK(alListener3f(AL_VELOCITY, v.x, v.y, 0.f));
#endif
	}

	void set_listener_orientation(const std::array<float, 6> data) {
		AL_CHECK(alListenerfv(AL_ORIENTATION, data.data()));
	}
}
