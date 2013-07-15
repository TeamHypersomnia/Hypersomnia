#include "timer.h"

namespace augmentations {
	namespace util {
		timer::timer() {
			QueryPerformanceFrequency(&freq);
			ticks.QuadPart = 0;
		}
		double timer::microseconds() {
			LARGE_INTEGER cmp;
			QueryPerformanceCounter(&cmp);
			delta.QuadPart = cmp.QuadPart - ticks.QuadPart;
			ticks.QuadPart = cmp.QuadPart;
			return delta.QuadPart * (1000000.0/freq.QuadPart);
		}

		double timer::miliseconds() {
			return microseconds() * 0.001;
		}
		double timer::seconds() { 
			return microseconds() * 0.000001;
		}

		double timer::get_microseconds() const {
			LARGE_INTEGER cmp;
			QueryPerformanceCounter(&cmp);
			return (cmp.QuadPart - ticks.QuadPart) * (1000000.0/freq.QuadPart);
		}

		double timer::get_miliseconds() const {
			return get_microseconds() * 0.001;
		}

		double timer::get_seconds() const { 
			return get_microseconds() * 0.000001;
		}

		fpstimer::fpstimer() : secs(0.0), sumframes(0.0), maxfps(1.0/1000.0){}

		void fpstimer::set_max_fps(double fps) {
			maxfps = 1.0/(fps = max(fps, 1.0));
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
			seconds();
		}

		void fpstimer::loop() {
			secs = seconds();
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