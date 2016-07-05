#pragma once
#include "ensure.h"

namespace augs {
	class fixed_delta;

	struct stepped_timestamp {
		unsigned long long step = 0;

		stepped_timestamp operator-(stepped_timestamp b) const;

		double in_seconds(fixed_delta) const;
		double in_milliseconds(fixed_delta) const;
	};

	struct stepped_timeout {
		stepped_timestamp when_started;

		bool is_set = false;
		float timeout_duration_ms = 1000.f;

		void unset();
		void set(float timeout_duration_ms, fixed_delta);
		bool passed(fixed_delta) const;
		bool lasts(fixed_delta) const;
	};

	struct stepped_cooldown {
		stepped_timestamp when_last_fired;

		bool ready_to_fire = true;
		float cooldown_duration_ms = 1000.f;

		stepped_cooldown(float cooldown_duration_ms);

		bool is_ready(fixed_delta t) const;
		bool try_to_fire_and_reset(fixed_delta t);
	};

}