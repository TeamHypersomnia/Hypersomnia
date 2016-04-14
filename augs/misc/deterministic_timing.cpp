#include "deterministic_timing.h"

namespace augs {
	deterministic_timeout::deterministic_timeout(float timeout_ms) : timeout_ms(timeout_ms) {}

	void deterministic_timeout::unset() {
		set(0);
	}

	void deterministic_timeout::set(float timeout_ms) {
		*this = deterministic_timeout(timeout_ms);
	}

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
}