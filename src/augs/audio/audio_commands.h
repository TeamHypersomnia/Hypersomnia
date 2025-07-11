#pragma once
#include <cstddef>
#include "augs/math/vec2.h"
#include "augs/math/si_scaling.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source_proxy_id.h"
#include "augs/audio/distance_model.h"

namespace augs {
	struct sound_source_proxy_data;
	class sound_buffer;

	enum class source_no_arg_command_type {
		PLAY,
		STOP
	};

	enum class source1f_command_type {
		GAIN,
		PITCH
	};

	struct update_listener_properties {
		si_scaling si;
		vec2 position;
		vec2 velocity;
		vec2 orientation;
	};

	struct update_multiple_properties {
		sound_source_proxy_id proxy_id;

		si_scaling si;
		vec2 position;
		vec2 velocity;
		float gain;
		float pitch;
		float doppler_factor;
		float reference_distance;
		float max_distance;
		float air_absorption_factor = 0.f;
		distance_model model;

		float lowpass_gainhf = -1.f;

		bool set_velocity = false;
		bool looping;
		bool is_direct_listener;

		void update(sound_source_proxy_data&);
	};

	struct source1f_command {
		sound_source_proxy_id proxy_id;
		source1f_command_type type;
		float v;
	};

	struct source_no_arg_command {
		sound_source_proxy_id proxy_id;
		source_no_arg_command_type type;
	};

	struct bind_sound_buffer {
		sound_source_proxy_id proxy_id;
		const sound_buffer* buffer = nullptr;
		std::size_t variation_index;
	};

	struct update_flash_noise {
		const sound_buffer* buffer = nullptr;
		float gain;
	};

	struct reseek_to_sync_if_needed {
		sound_source_proxy_id proxy_id;

		float expected_secs;
		float max_divergence;
	};
}
