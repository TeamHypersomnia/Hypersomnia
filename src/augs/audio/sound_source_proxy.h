#pragma once
#include "augs/audio/audio_renderer.h"
#include "augs/audio/sound_source_proxy_id.h"

namespace augs {
	class sound_buffer;

	struct sound_source_proxy_data {
		sound_source_proxy_id id = 0;
		sound_buffer_meta buffer_meta;

		float last_pitch = 0.f;
		float last_gain = 0.f;

		sound_source_proxy_data() = default;
		sound_source_proxy_data(const sound_source_proxy_id& id) : id(id) {}
	};

	class sound_source_proxy {
	public:
		const audio_renderer& commands;
		sound_source_proxy_data& proxy;

		void play() const {
			commands.push_command(source_no_arg_command { proxy.id, source_no_arg_command_type::PLAY });
		}

		void stop() const {
			commands.push_command(source_no_arg_command { proxy.id, source_no_arg_command_type::STOP });
		}

		void set_gain(const float v) const {
			commands.push_command(source1f_command { proxy.id, source1f_command_type::GAIN, v });
			proxy.last_gain = v;
		}

		void set_pitch(const float v) const {
			commands.push_command(source1f_command { proxy.id, source1f_command_type::PITCH, v });
			proxy.last_pitch = v;
		}

		float get_gain() const {
			return proxy.last_gain;
		}

		float get_pitch() const {
			return proxy.last_pitch;
		}

		void bind_buffer(
			const sound_buffer& buf, 
			const std::size_t variation_index
		) const {
			commands.push_command(bind_sound_buffer { proxy.id, std::addressof(buf), variation_index });
			proxy.buffer_meta = buf.get_buffer(variation_index).get_meta();
		}
	};
}