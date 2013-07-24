#pragma once
#include <ratio>
#include <chrono>

namespace augmentations {
	namespace util {
		class timer {
			std::chrono::high_resolution_clock::time_point ticks;
		public:

			/* returns time that has lasted since last call to "extract" (zeroes the ticks) */
			template<class resolution>
			double extract() {
				using namespace std::chrono;

				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				high_resolution_clock::now();
				auto now = high_resolution_clock::now();
				double count = duration_cast<duration<double, resolution::period>>(now - ticks).count();
				ticks = now;
				return count;
			}
			
			/* returns time that has lasted since last call to "extract" */
			template<class resolution>
			double get() const {
				using namespace std::chrono;
				return duration_cast<duration<double, resolution::period>>(high_resolution_clock::now() - ticks).count();
			}

			void reset();
			timer();
		};

		/* WARNING! This variable timestep timer should be replaced with delta accumulation functionality! */
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
