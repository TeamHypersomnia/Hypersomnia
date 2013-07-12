#pragma once
#define UNICODE
#include <Windows.h>

namespace augmentations {
	namespace util {
		class timer {
			LARGE_INTEGER freq, ticks, cmp, delta;
		public:
			double microseconds();
			double miliseconds();
			double seconds();
			double get_microseconds();
			double get_miliseconds();
			double get_seconds();
			timer();
		};

		class fpstimer : private timer {
			double maxfps;
			double sumframes, secs;
		public:
			void set_max_fps(double);
			double get_max_fps();
			void start();
			void loop();
			void reset();
			bool render();
			double frame_speed();
			double loop_speed();
			double get_frame_rate();
			double get_loop_rate();
			double speed_factor();

			fpstimer();
		};
	}
}
