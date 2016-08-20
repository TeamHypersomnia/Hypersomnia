#include "stepped_timing.h"
#include "delta.h"

namespace augs {
	stepped_timestamp stepped_timestamp::operator-(stepped_timestamp b) const {
		stepped_timestamp stamp;
		stamp.step = step - b.step;
		return stamp;
	}

	float stepped_timestamp::in_seconds(fixed_delta delta) const {
		return step * delta.in_seconds();
	}

	float stepped_timestamp::in_milliseconds(fixed_delta delta) const {
		return step * delta.in_milliseconds();
	}

	stepped_cooldown::stepped_cooldown(float cooldown_duration_ms) : ready_to_fire(true), cooldown_duration_ms(cooldown_duration_ms) {}

	bool stepped_cooldown::is_ready(stepped_timestamp s, fixed_delta t) const {
		return ready_to_fire || (s - when_last_fired).in_milliseconds(t) > cooldown_duration_ms;
	}

	bool stepped_cooldown::try_to_fire_and_reset(stepped_timestamp s, fixed_delta t) {
		if (is_ready(s, t)) {
			ready_to_fire = false;
			when_last_fired = s;
			return true;
		}

		return false;
	}

	void stepped_timeout::unset() {
		is_set = false;
	}

	void stepped_timeout::set(float duration_ms, stepped_timestamp s) {
		timeout_duration_ms = duration_ms;
		when_started = s;
	}

	bool stepped_timeout::passed(stepped_timestamp s, fixed_delta t) const {
		return !is_set || (s - when_started).in_milliseconds(t) > timeout_duration_ms;
	}

	bool stepped_timeout::lasts(stepped_timestamp s, fixed_delta t) const {
		return !passed(s, t);
	}
}