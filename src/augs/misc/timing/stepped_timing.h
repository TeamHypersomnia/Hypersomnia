#pragma once
#include "augs/misc/timing/delta.h"

namespace augs {
	class delta;

	struct stepped_timestamp {
		// GEN INTROSPECTOR struct augs::stepped_timestamp
		unsigned step = static_cast<unsigned>(-1);
		// END GEN INTROSPECTOR

		stepped_timestamp operator-(const stepped_timestamp b) const;

		bool operator>=(const stepped_timestamp) const;
		bool operator==(const stepped_timestamp) const;
		bool operator!=(const stepped_timestamp) const;

		float in_seconds(const delta) const;
		float in_milliseconds(const delta) const;

		bool was_set() const;
	};

	struct stepped_clock {
		// GEN INTROSPECTOR struct augs::stepped_clock
		stepped_timestamp now;
		delta dt = delta::steps_per_second(60);
		// END GEN INTROSPECTOR

		float diff_seconds(const stepped_clock& lesser) const {
			/* TODO: Account for different deltas when they can change. */
			return (now - lesser.now).in_seconds(dt);
		}

		template <class T>
		bool is_ready(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			return !stamp.was_set() || (now - stamp).in_milliseconds(dt) > cooldown_ms;
		}

		template <class T>
		bool lasts(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			return !is_ready(cooldown_ms, stamp);
		}

		template <class T>
		bool try_to_fire_and_reset(
			const T cooldown_ms, 
			stepped_timestamp& stamp
		) const {
			if (is_ready(cooldown_ms, stamp)) {
				stamp = now;
				return true;
			}

			return false;
		}

		template <class T>
		auto get_ratio_of_remaining_time(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			if (!stamp.was_set()) {
				return 0.f;
			}

			return 1.f - ((now - stamp).in_milliseconds(dt) / cooldown_ms);
		}

		template <class T>
		auto get_remaining_time_ms(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			if (!stamp.was_set()) {
				return 0.f;
			}

			return cooldown_ms - (now - stamp).in_milliseconds(dt);
		}
	};

	struct stepped_cooldown {
		// GEN INTROSPECTOR struct augs::stepped_cooldown
		stepped_timestamp when_last_fired;
		float cooldown_duration_ms = 1000.f;
		// END GEN INTROSPECTOR

		stepped_cooldown(const float cooldown_duration_ms = 1000.f);
		void set(const float cooldown_duration_ms, const stepped_timestamp now);
		
		float get_remaining_time_ms(const stepped_clock&) const;
		float get_ratio_of_remaining_time(const stepped_clock&) const;

		bool lasts(const stepped_clock&) const;
		bool is_ready(const stepped_clock&) const;
		bool try_to_fire_and_reset(const stepped_clock&);

		bool operator==(const stepped_cooldown& b) const {
			return when_last_fired == b.when_last_fired && cooldown_duration_ms == b.cooldown_duration_ms;
		}
	};

}
