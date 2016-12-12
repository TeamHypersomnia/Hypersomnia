#pragma once
#include "augs/math/vec2.h"

typedef unsigned int ALuint;

namespace augs {
	class single_sound_buffer;

	void set_listener_velocity(vec2);
	void set_listener_position(vec2);
	void set_listener_orientation(const std::array<float, 6>);

	class sound_source {
		sound_source(const sound_source&) = delete;
		sound_source& operator=(const sound_source&) = delete;

		bool initialized = false;
		ALuint id = 0;
		const single_sound_buffer* attached_buffer = nullptr;
	public:
		sound_source();
		~sound_source();

		sound_source(sound_source&&);
		sound_source& operator=(sound_source&&);

		void play() const;
		void stop() const;
		void set_looping(const bool) const;
		void set_pitch(const float) const;
		void set_gain(const float) const;
		void set_velocity(vec2) const;
		void set_position(vec2) const;
		void set_max_distance(const float) const;
		void set_reference_distance(const float) const;

		void bind_buffer(const single_sound_buffer&);
		void unbind_buffer();
		const single_sound_buffer* get_bound_buffer() const;

		ALuint get_id() const;
		operator ALuint() const;
	};
}