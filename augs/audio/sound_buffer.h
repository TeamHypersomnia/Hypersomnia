#pragma once

typedef unsigned int ALuint;

namespace augs {
	class sound_buffer {
		sound_buffer(const sound_buffer&) = delete;
		sound_buffer(sound_buffer&&) = delete;
		sound_buffer& operator=(const sound_buffer&) = delete;
		sound_buffer& operator=(sound_buffer&&) = delete;
		
		ALuint id = 0;
	public:
		sound_buffer();
		~sound_buffer();

		ALuint get_id() const;
		operator ALuint() const;
	};
}