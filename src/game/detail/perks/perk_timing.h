#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/timing/delta.h"

struct perk_timing {
	// GEN INTROSPECTOR struct perk_timing
	augs::stepped_cooldown duration;
	// END GEN INTROSPECTOR

	void set_for_duration(
		const float ms,
		const augs::stepped_timestamp now
	) {
		duration.set(ms, now);
	}

	float get_ratio(
		const augs::stepped_timestamp now,
		const augs::delta dt
	) const {
		return duration.get_ratio_of_remaining_time(now, dt);
	}

	bool is_enabled(
		const augs::stepped_timestamp now,
		const augs::delta dt
	) const {
		return duration.lasts(now, dt);
	}
};