#include "deterministic_timing.h"
#include "delta.h"

namespace augs {
	deterministic_timestamp deterministic_timestamp::operator-(deterministic_timestamp b) const {
		ensure(seconds_passed >= b.seconds_passed);

		deterministic_timestamp result = *this;
		result.seconds_passed -= b.seconds_passed;

		return result;
	}

	deterministic_timestamp::operator double() const {
		return get_seconds();
	}

	double deterministic_timestamp::get_milliseconds() const {
		return get_seconds() * 1000.0;
	}

	double deterministic_timestamp::get_seconds() const {
		return seconds_passed;
	}

	bool deterministic_timeout::passed(fixed_delta t) const {
		return (parent_cosmos.current_step_number - step_recorded) * t.in_milliseconds() >= timeout_ms;
	}

	bool deterministic_timeout::unset_or_passed(fixed_delta t) const {
		return !t.was_set || passed(t);
	}

	bool deterministic_timeout::was_set_and_passed(fixed_delta t) const {
		return t.was_set && passed(t);
	}

	bool deterministic_timeout::check_timeout_and_reset(fixed_delta t) {
		if (unset_or_passed(t)) {
			reset(t);
			return true;
		}

		return false;
	}

	void deterministic_timeout::reset(augs::deterministic_timeout& t) {
		t.step_recorded = parent_cosmos.current_step_number;
		t.was_set = true;
	}
}