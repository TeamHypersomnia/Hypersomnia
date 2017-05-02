#include "delta.h"
#include "stepped_timing.h"

namespace augs {
	float delta::in_seconds() const {
		return delta_ms / 1000.f;
	}

	float delta::in_milliseconds() const {
		return delta_ms;
	}
}