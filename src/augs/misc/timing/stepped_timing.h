#pragma once
#include "augs/misc/timing/delta.h"

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

		bool operator==(const stepped_cooldown& b) const {
			return when_last_fired == b.when_last_fired && cooldown_duration_ms == b.cooldown_duration_ms;
		}
	};

	template <class T>
	bool is_ready(
		const T cooldown_ms, 
		const stepped_timestamp stamp, 
		const stepped_timestamp now,
		const delta dt
	) {
		return !stamp.step || (now - stamp).in_milliseconds(dt) > cooldown_ms;
	}

	template <class T>
	bool lasts(
		const T cooldown_ms, 
		const stepped_timestamp stamp, 
		const stepped_timestamp now,
		const delta dt
	) {
		return !is_ready(cooldown_ms, stamp, now, dt);
	}

	template <class T>
	bool try_to_fire_and_reset(
		const T cooldown_ms, 
		stepped_timestamp& stamp, 
		const stepped_timestamp now,
		const delta dt
	) {
		if (is_ready(cooldown_ms, stamp, now, dt)) {
			stamp = now;
			return true;
		}

		return false;
	}

	template <class T>
	auto get_ratio_of_remaining_time(
		const T cooldown_ms, 
		const stepped_timestamp stamp, 
		const stepped_timestamp now,
		const delta dt
	) {
		if (!stamp.step) {
			return 0.f;
		}

		return 1.f - ((now - stamp).in_milliseconds(dt) / cooldown_ms);
	}

	template <class T>
	auto get_remaining_time_ms(
		const T cooldown_ms, 
		const stepped_timestamp stamp, 
		const stepped_timestamp now,
		const delta dt
	) {
		if (!stamp.step) {
			return 0.f;
		}

		return cooldown_ms - (now - stamp).in_milliseconds(dt);
	}
}
