#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/timing/stepped_timing.h"

namespace components {
	struct sentience;
};

struct sentience_shake {
	// GEN INTROSPECTOR struct sentience_shake
	real32 duration_ms = 0.f;
	real32 mult = 1.f;
	// END GEN INTROSPECTOR

	bool any() const {
		return duration_ms > 0.f;
	}

	void apply(const augs::stepped_timestamp now, components::sentience& sentience) const;
};
