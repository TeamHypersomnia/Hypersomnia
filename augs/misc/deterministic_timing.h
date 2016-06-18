#pragma once
#include "ensure.h"

namespace augs {
	struct deterministic_timeout {
		unsigned long long step_recorded = 0;
		bool was_set = false;
		float timeout_ms = 1000.f;

		void unset();
		void set(float timeout_ms);
		deterministic_timeout(float timeout_ms);
	};

	struct deterministic_timestamp {
		double seconds_passed = -1;

		deterministic_timestamp operator-(deterministic_timestamp b) const;

		operator double() const;

		double get_milliseconds() const;
		double get_seconds() const;
	};
}