#pragma once
#include <array>
#include <stdexcept>

#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"
#include "augs/audio/distance_model.h"
#include "augs/audio/sound_buffer_structs.h"

typedef unsigned int ALuint;

namespace augs {
	struct too_many_sound_sources_error : std::runtime_error {
		explicit too_many_sound_sources_error() 
			: std::runtime_error("Too many sound sources. This should never happen.") {}
	};
}

namespace augs {
	class single_sound_buffer;
	class sound_buffer;

	void set_listener_velocity(const si_scaling, vec2);
	void set_listener_position(const si_scaling, vec2);
	void set_listener_orientation(const std::array<float, 6>);

	class sound_source {
		bool initialized = false;
		ALuint id = 0;
		ALuint attached_buffer = -1;
		sound_buffer_meta buffer_meta;

		void destroy();
	public:
		sound_source();
		~sound_source();

		sound_source(sound_source&&);
		sound_source& operator=(sound_source&& b);

		sound_source(const sound_source&) = delete;
		sound_source& operator=(const sound_source& b) = delete;

		void set_relative(bool) const;
		void set_relative_and_zero_vel_pos() const;

		void play() const;
		void seek_to(float seconds) const;
		void stop() const;
		void set_looping(bool) const;
		void set_doppler_factor(float) const;
		void set_rolloff_factor(float) const;
		void set_pitch(float) const;
		void set_gain(float) const;
		void set_air_absorption_factor(float) const;
		void set_velocity(si_scaling, vec2) const;
		void set_position(si_scaling, vec2) const;
		void set_max_distance(si_scaling, float) const;
		void set_reference_distance(si_scaling, float) const;
		void set_direct_channels(bool) const;
		void set_distance_model(distance_model);
		void set_spatialize(bool) const;

		float get_time_in_seconds() const;
		float get_gain() const;
		float get_pitch() const;
		bool is_playing() const;

		void bind_buffer(const single_sound_buffer&);

		void bind_buffer(
			const sound_buffer&, 
			std::size_t variation_index
		);

		void just_play(const single_sound_buffer&, float gain = -1.f);

		void unbind_buffer();

		const sound_buffer_meta& get_bound_buffer_meta() const {
			return buffer_meta;
		}

		ALuint get_id() const;
		operator ALuint() const;
	};
}