#pragma once
#include "augs/ensure.h"

namespace augs {
	class delta;

	struct stepped_timestamp {
		// GEN INTROSPECTOR struct augs::stepped_timestamp
		unsigned step;
		// END GEN INTROSPECTOR

		stepped_timestamp(const unsigned step = 0u)
			: step(step) {}

		stepped_timestamp operator-(const stepped_timestamp b) const;

		bool operator>=(const stepped_timestamp) const;
		bool operator==(const stepped_timestamp) const;
		bool operator!=(const stepped_timestamp) const;

		float in_seconds(const delta) const;
		float in_milliseconds(const delta) const;

		bool was_set() const;
	};

	struct stepped_cooldown {
		// GEN INTROSPECTOR struct augs::stepped_cooldown
		stepped_timestamp when_last_fired;
		float cooldown_duration_ms = 1000.f;
		// END GEN INTROSPECTOR

		stepped_cooldown(const float cooldown_duration_ms = 1000.f);
		void set(const float cooldown_duration_ms, const stepped_timestamp now);
		
		float get_remaining_time_ms(const stepped_timestamp, const delta t) const;
		float get_ratio_of_remaining_time(const stepped_timestamp, const delta t) const;

		bool lasts(const stepped_timestamp, const delta t) const;
		bool is_ready(const stepped_timestamp, const delta t) const;
		bool try_to_fire_and_reset(const stepped_timestamp, const delta t);
	};

}
