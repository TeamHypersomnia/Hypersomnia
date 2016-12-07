#pragma once

typedef unsigned int ALuint;

namespace augs {
	class sound_source {
		sound_source(const sound_source&) = delete;
		sound_source& operator=(const sound_source&) = delete;

		bool initialized = false;
		ALuint id = 0;
	public:
		sound_source();
		~sound_source();

		sound_source(sound_source&&);
		sound_source& operator=(sound_source&&);

		ALuint get_id() const;
		operator ALuint() const;
	};
}