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

	float get_ratio(const augs::stepped_clock& clk) const {
		return duration.get_ratio_of_remaining_time(clk);
	}

	bool is_enabled(const augs::stepped_clock& clk) const {
		return duration.lasts(clk);
	}
};