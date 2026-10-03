#pragma once
#include <cstdint>
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

	/*
		tickrate - how many steps per real second the client/server performs.
		logic_speed - how much logic time passes per real time.

		dt - the logic delta passed to the solver: (1 / tickrate) * logic_speed.
		Everything measured with dt (physics, movement, fuses, reloads, AI) slows down with logic_speed.

		Durations that should stay constant in real time regardless of logic_speed
		(fire rates, recoil, mode timers) should use get_real_dt() or get_real_clock().

		Only change tickrate and logic_speed through set_timing so that dt stays consistent.
	*/

	struct stepped_clock {
		// GEN INTROSPECTOR struct augs::stepped_clock
		delta dt = delta::steps_per_second(60);
		uint32_t tickrate = 60;
		real32 logic_speed = 1.f;
		stepped_timestamp now = { static_cast<unsigned>(0) };
		// END GEN INTROSPECTOR

		static stepped_clock from_timestamp(const stepped_clock& source, const stepped_timestamp new_now) {
			auto result = source;
			result.now = new_now;
			return result;
		}

		void set_timing(const uint32_t new_tickrate, const real32 new_logic_speed) {
			tickrate = new_tickrate;
			logic_speed = new_logic_speed;
			dt = delta::steps_per_second(tickrate);

			if (logic_speed != 1.f) {
				dt *= logic_speed;
			}
		}

		delta get_real_dt() const {
			return delta::steps_per_second(tickrate);
		}

		/*
			A copy of this clock whose dt measures real time.
			Pass it to systems whose cooldowns should not slow down with logic_speed.
		*/

		stepped_clock get_real_clock() const {
			auto result = *this;
			result.set_timing(tickrate, 1.f);
			return result;
		}

		template <class T>
		real32 logic_to_real_secs(const T logic_secs) const {
			return static_cast<real32>(logic_secs) / logic_speed;
		}

		template <class T>
		real32 logic_to_real_ms(const T logic_ms) const {
			return static_cast<real32>(logic_ms) / logic_speed;
		}

		auto diff_real_seconds(const stepped_clock& lesser) const {
			return (now - lesser.now).in_seconds(get_real_dt());
		}

		double get_real_seconds_passed() const {
			return now.step * get_real_dt().in_seconds<double>();
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
