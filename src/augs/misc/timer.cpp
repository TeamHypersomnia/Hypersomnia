#include <algorithm>
#include <numeric>

#include "augs/misc/timer.h"

using namespace std::chrono;

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		ticks = high_resolution_clock::now();
		paused_difference = paused_difference.zero();
		is_paused = false;
	}

	void timer::pause(const bool flag) {
		if (!is_paused && flag) {
			paused_difference += duration_cast<duration<system_clock::rep, system_clock::period>>(high_resolution_clock::now() - ticks);
		}
		else if (is_paused && !flag) {
			ticks = high_resolution_clock::now();
		}

		is_paused = flag;
	}
}