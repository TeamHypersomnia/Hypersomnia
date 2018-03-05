#include "augs/misc/timing/timer.h"

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		ticks = now();
	}
}