#pragma once
#include "ensure.h"

namespace augs {
	struct stepped_cooldown {
		unsigned long long step_recorded = 0;

		bool ready = true;
		float cooldown_duration_ms = 1000.f;

		bool passed(fixed_delta t) const;
		bool unset_or_passed(fixed_delta t) const;
		bool was_set_and_passed(fixed_delta t) const;
		bool check_for_readyness_and_reset(fixed_delta t);
		void reset(fixed_delta t);

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