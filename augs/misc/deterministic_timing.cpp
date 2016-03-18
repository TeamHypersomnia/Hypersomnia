#include "deterministic_timing.h"

namespace augs {
	deterministic_timeout::deterministic_timeout(float timeout_ms) : timeout_ms(timeout_ms) {}

	void deterministic_timeout::unset() {
		set(0);
	}

	void deterministic_timeout::set(float timeout_ms) {
		*this = deterministic_timeout(timeout_ms);
	}
}