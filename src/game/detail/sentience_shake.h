#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/timing/stepped_timing.h"

namespace components {
	struct sentience;
};

namespace invariants {
	struct sentience;
};

struct sentience_shake_settings {
	// GEN INTROSPECTOR struct sentience_shake_settings
	real32 final_mult = 1.f;
	real32 duration_mult = 1.f;
	real32 duration_unit = 1500.f;
	real32 max_duration_ms = 500.f;
	// END GEN INTROSPECTOR
};

struct sentience_shake {
	// GEN INTROSPECTOR struct sentience_shake
	real32 duration_ms = 0.f;
	real32 mult = 1.f;
	// END GEN INTROSPECTOR

	static auto zero() {
		sentience_shake out;
		out.duration_ms = 0.f;
		out.mult = 0.f;
		return out;
	}

	bool any() const {
		return duration_ms > 0.f;
	}

	void apply(
		const augs::stepped_timestamp now, 
		const invariants::sentience& sentience_def,
		components::sentience& sentience
	) const;

	auto& operator*=(const real32 scalar) {
		duration_ms *= scalar;
		mult *= scalar;

		return *this;
	}
};
