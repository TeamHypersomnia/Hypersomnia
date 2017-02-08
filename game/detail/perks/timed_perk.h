#pragma once
#include "augs/misc/stepped_timing.h"
#include "augs/misc/delta.h"

struct timed_perk {
	augs::stepped_cooldown duration;

	void set_for_duration(
		const float ms,
		const augs::stepped_timestamp now
	) {
		duration.set(ms, now);
	}

	bool is_enabled(
		const augs::stepped_timestamp now,
		const augs::delta dt
	) const {
		return duration.lasts(now, dt);
	}
};