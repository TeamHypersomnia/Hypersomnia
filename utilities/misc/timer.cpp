#include "timer.h"
#include <algorithm>


namespace augs {
	namespace misc {
		timer::timer() {
			reset();
		}

		void timer::reset() {
			ticks = std::chrono::high_resolution_clock::now();
		}

		fpstimer::fpstimer() : secs(0.0), sumframes(0.0), maxfps(1.0/1000.0){}

		void fpstimer::set_max_fps(double fps) {
			maxfps = 1.0/(fps = std::max(fps, 1.0));
		}

		double fpstimer::get_max_fps() {
			return 1.0/maxfps;
		}

		double fpstimer::get_frame_rate() {
			return 1.0/frame_speed();
		}

		double fpstimer::get_loop_rate() {
			return 1.0/loop_speed();
		}

		double fpstimer::speed_factor() {
			return maxfps * frame_speed();
		}

		void fpstimer::start() {
			reset();
		}

		void fpstimer::loop() {
			secs = extract<std::chrono::seconds>();
			sumframes += secs;
		}

		bool fpstimer::render() {
			return sumframes > maxfps;
		}

		void fpstimer::reset() {
			sumframes = 0.0;
		}

		double fpstimer::frame_speed() {
			return sumframes;
		}

		double fpstimer::loop_speed() {
			return secs;
		}
	}
}