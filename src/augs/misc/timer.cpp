#include "augs/misc/timer.h"

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		ticks = now();
	}
}