#include "timer.h"
#include <algorithm>
#include <numeric>
#include "augs/log.h"

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		ticks = std::chrono::high_resolution_clock::now();
		paused_difference = paused_difference.zero();
		is_paused = false;
	}

	void timer::pause(bool flag) {
		if (!is_paused && flag) {
			paused_difference += std::chrono::duration_cast<std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period>>(std::chrono::high_resolution_clock::now() - ticks);
		}
		else if (is_paused && !flag) {
			ticks = std::chrono::high_resolution_clock::now();
		}

		is_paused = flag;
	}



}