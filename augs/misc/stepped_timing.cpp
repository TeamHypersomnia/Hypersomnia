#include "stepped_timing.h"
#include "delta.h"

namespace augs {
	stepped_timestamp stepped_timestamp::operator-(const stepped_timestamp b) const {
		stepped_timestamp stamp;
		stamp.step = step - b.step;
		return stamp;
	}


	bool stepped_timestamp::operator==(const stepped_timestamp b) const {
		return step == b.step;
	}

	bool stepped_timestamp::operator!=(const stepped_timestamp b) const {
		return !operator==(b);
	}

	float stepped_timestamp::in_seconds(const delta delta) const {
		return step * delta.in_seconds();
	}

	float stepped_timestamp::in_milliseconds(const delta delta) const {
		return step * delta.in_milliseconds();
	}

	stepped_cooldown::stepped_cooldown(const float cooldown_duration_ms) : cooldown_duration_ms(cooldown_duration_ms) {}

	void stepped_cooldown::set(const float ms, const stepped_timestamp now) {
		cooldown_duration_ms = ms;
		when_last_fired = now;
	}

	bool stepped_cooldown::is_ready(
		const stepped_timestamp s,
		const delta t
	) const {
		return !when_last_fired.step || (s - when_last_fired).in_milliseconds(t) > cooldown_duration_ms;
	}

	bool stepped_cooldown::lasts(
		const stepped_timestamp s,
		const delta t
	) const {
		return !is_ready(s, t);
	}

	bool stepped_cooldown::try_to_fire_and_reset(
		const stepped_timestamp s, 
		const delta t
	) {
		if (is_ready(s, t)) {
			when_last_fired = s;
			return true;
		}

		return false;
	}
}