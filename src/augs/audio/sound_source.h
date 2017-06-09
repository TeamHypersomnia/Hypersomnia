#pragma once
#include "augs/math/vec2.h"

typedef unsigned int ALuint;
struct si_scaling;

namespace augs {
	class single_sound_buffer;

	void set_listener_velocity(const si_scaling, vec2);
	void set_listener_position(const si_scaling, vec2);
	void set_listener_orientation(const std::array<float, 6>);

	class sound_source {
		bool initialized = false;
		ALuint id = 0;
		const single_sound_buffer* attached_buffer = nullptr;

		void destroy();
	public:
		friend void swap(sound_source& a, sound_source& b) {
			using std::swap;

			swap(a.attached_buffer, b.attached_buffer);
			swap(a.initialized, b.initialized);
			swap(a.id, b.id);
		}

		sound_source();
		~sound_source();

		sound_source(sound_source&& b) 
			: initialized(false) {
			swap(*this, b);
		}
		
		sound_source& operator=(sound_source b) {
			swap(*this, b);
			return *this;
		}

		sound_source(const sound_source&) = delete;

		void play() const;
		void seek_to(const float seconds) const;
		void stop() const;
		void set_looping(const bool) const;
		void set_pitch(const float) const;
		void set_gain(const float) const;
		void set_air_absorption_factor(const float) const;
		void set_velocity(const si_scaling, vec2) const;
		void set_position(const si_scaling, vec2) const;
		void set_max_distance(const si_scaling, const float) const;
		void set_reference_distance(const si_scaling, const float) const;
		void set_direct_channels(const bool) const;

		float get_time_in_seconds() const;
		float get_gain() const;
		float get_pitch() const;
		bool is_playing() const;

		void bind_buffer(const single_sound_buffer&);
		void unbind_buffer();
		const single_sound_buffer* get_bound_buffer() const;

		ALuint get_id() const;
		operator ALuint() const;
	};
}