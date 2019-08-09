#pragma once
#include "augs/audio/audio_command.h"

namespace augs {
	class audio_renderer {
	public:
		audio_command_buffer& commands;

		template <class T>
		void push_command(T&& t) const {
			commands.emplace_back(audio_command { std::forward<T>(t) });
		}
	};
}
