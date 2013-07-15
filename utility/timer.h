#pragma once
#define UNICODE
#include <Windows.h>

namespace augmentations {
	namespace util {
		class timer {
			LARGE_INTEGER freq, ticks, delta;
		public:
			/* returns time that has lasted since last call to "microseconds" (zeroes the ticks) */
			double microseconds();
			/* returns time in miliseconds that has lasted since last call to "microseconds" (zeroes the ticks) */
			double miliseconds();
			/* returns time in seconds that has lasted since last call to "microseconds" (zeroes the ticks) */
			double seconds();
			
			/* returns time that has lasted since last call to "microseconds" */
			double get_microseconds() const;
			/* returns time in miliseconds that has lasted since last call to "microseconds" */
			double get_miliseconds() const;
			/* returns time in seconds that has lasted since last call to "microseconds" */
			double get_seconds() const;
			
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
