#include "augs/misc/timing/timer.h"

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		const auto now = std::chrono::high_resolution_clock::now();
		ticks = now;
	}
}