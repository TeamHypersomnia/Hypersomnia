#include "stepped_timing.h"
#include "delta.h"

namespace augs {
	stepped_timestamp stepped_timestamp::operator-(stepped_timestamp b) const {
		stepped_timestamp stamp;
		stamp.step = step - b.step;
		return stamp;
	}

	double stepped_timestamp::in_seconds(fixed_delta delta) const {
		return step * delta.in_seconds();
	}

	double stepped_timestamp::in_milliseconds(fixed_delta delta) const {
		return step * delta.in_milliseconds();
	}

	stepped_cooldown::stepped_cooldown(float cooldown_duration_ms) : ready_to_fire(true), cooldown_duration_ms(cooldown_duration_ms) {}

	bool stepped_cooldown::is_ready(fixed_delta t) const {
		return ready_to_fire || (t.get_timestamp() - when_last_fired).in_milliseconds(t) > cooldown_duration_ms;
	}

	bool stepped_cooldown::try_to_fire_and_reset(fixed_delta t) {
		if (is_ready(t)) {
			ready_to_fire = false;
			when_last_fired = t.get_timestamp();
			return true;
		}

		return false;
	}

	void stepped_timeout::unset() {
		is_set = false;
	}

	void stepped_timeout::set(float duration_ms, fixed_delta t) {
		timeout_duration_ms = duration_ms;
		when_started = t.get_timestamp();
	}

	bool stepped_timeout::passed(fixed_delta t) const {
		return !is_set || (t.get_timestamp() - when_started).in_milliseconds(t) > timeout_duration_ms;
	}

	bool stepped_timeout::lasts(fixed_delta t) const {
		return !passed(t);
	}
}