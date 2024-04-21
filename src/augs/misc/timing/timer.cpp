#include "augs/misc/timing/timer.h"

double yojimbo_time();

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		const auto now = std::chrono::high_resolution_clock::now();
		ticks = now;
	}

	double high_precision_secs() {
#if PLATFORM_WEB
		auto now = std::chrono::high_resolution_clock::now();
		auto since_epoch = now.time_since_epoch();
		return std::chrono::duration<double>(since_epoch).count();
#else
		return yojimbo_time();
#endif
	}
}