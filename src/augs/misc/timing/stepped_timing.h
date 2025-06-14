#pragma once
#include "augs/misc/timing/delta.h"
#include "augs/pad_bytes.h"

namespace augs {
	class delta;

	struct stepped_timestamp {
		// GEN INTROSPECTOR struct augs::stepped_timestamp
		uint32_t step = static_cast<uint32_t>(-1);
		// END GEN INTROSPECTOR

		stepped_timestamp operator-(const stepped_timestamp b) const;

		bool operator<(const stepped_timestamp) const;
		bool operator>(const stepped_timestamp) const;
		bool operator>=(const stepped_timestamp) const;
		bool operator==(const stepped_timestamp) const;
		bool operator!=(const stepped_timestamp) const;

		real32 in_seconds(const delta) const;
		real32 in_milliseconds(const delta) const;

		bool was_set() const;
	};

	using real_cooldown = real32;

	struct stepped_clock {
		// GEN INTROSPECTOR struct augs::stepped_clock
		delta dt = delta::steps_per_second(60);
		stepped_timestamp now = { static_cast<unsigned>(0) };
		// END GEN INTROSPECTOR

		auto diff_seconds(const stepped_clock& lesser) const {
			/* TODO: Account for different deltas when they can change. */
			return (now - lesser.now).in_seconds(dt);
		}

		bool was_set() const {
			return now.was_set();
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

		auto get_passed_ms(const stepped_timestamp stamp) const {
			return (now - stamp).in_milliseconds(dt);
		}
		
		auto get_passed_secs(const stepped_timestamp stamp) const {
			return (now - stamp).in_seconds(dt);
		}

		template <class T>
		auto get_remaining_ms(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			if (!stamp.was_set()) {
				return 0.f;
			}

			return cooldown_ms - (now - stamp).in_milliseconds(dt);
		}

		template <class T>
		auto get_remaining_secs(
			const T cooldown_ms, 
			const stepped_timestamp stamp
		) const {
			return get_remaining_ms(cooldown_ms, stamp) / 1000;
		}

		template <class T>
		bool is_ready(
			const T cooldown_ms, 
			const real_cooldown current_cooldown_ms
		) const {
			(void)cooldown_ms;

			return current_cooldown_ms <= 0.f;
		}

		template <class T>
		bool try_to_fire_and_reset(
			const T cooldown_ms, 
			real_cooldown& current_cooldown_ms
		) const {
			if (current_cooldown_ms <= 0.f) {
				current_cooldown_ms += cooldown_ms;
				return true;
			}

			return false;
		}

		template <class T>
		auto get_passed_ms(
			const T cooldown_ms,
			const real_cooldown current_cooldown_ms
		) const {
			return cooldown_ms - current_cooldown_ms;
		}

		template <class T>
		auto get_passed_secs(
			const T cooldown_ms,
			const real_cooldown current_cooldown_ms
		) const {
			return (cooldown_ms - current_cooldown_ms) / 1000;
		}

		template <class T>
		auto get_ratio_of_remaining_time(
			const T cooldown_ms, 
			const real_cooldown current_cooldown_ms
		) const {
			return std::max(0.f, current_cooldown_ms / cooldown_ms);
		}
	};

	static_assert(sizeof(stepped_clock) == sizeof(delta) + sizeof(stepped_timestamp));

	struct stepped_cooldown {
		// GEN INTROSPECTOR struct augs::stepped_cooldown
		stepped_timestamp when_last_fired;
		real32 cooldown_duration_ms = 1000.f;
		// END GEN INTROSPECTOR

		stepped_cooldown(const real32 cooldown_duration_ms = 1000.f);
		void set(const real32 cooldown_duration_ms, const stepped_timestamp now);
		
		real32 get_remaining_ms(const stepped_clock&) const;
		real32 get_ratio_of_remaining_time(const stepped_clock&) const;

		bool lasts(const stepped_clock&) const;
		bool is_ready(const stepped_clock&) const;
		bool try_to_fire_and_reset(const stepped_clock&);

		bool operator==(const stepped_cooldown& b) const {
			return when_last_fired == b.when_last_fired && cooldown_duration_ms == b.cooldown_duration_ms;
		}
	};

}
