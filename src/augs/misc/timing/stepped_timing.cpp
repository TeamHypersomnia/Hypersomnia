#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/timing/delta.h"

namespace augs {
	stepped_timestamp stepped_timestamp::operator-(const stepped_timestamp b) const {
		stepped_timestamp stamp;
		stamp.step = step - b.step;
		return stamp;
	}

	bool stepped_timestamp::operator>=(const stepped_timestamp b) const {
		return step >= b.step;
	}

	bool stepped_timestamp::operator>(const stepped_timestamp b) const {
		return step > b.step;
	}

	bool stepped_timestamp::operator<(const stepped_timestamp b) const {
		return step < b.step;
	}

	bool stepped_timestamp::operator==(const stepped_timestamp b) const {
		return step == b.step;
	}

	bool stepped_timestamp::operator!=(const stepped_timestamp b) const {
		return !operator==(b);
	}

	real32 stepped_timestamp::in_seconds(const delta delta) const {
		return step * delta.in_seconds();
	}

	real32 stepped_timestamp::in_milliseconds(const delta delta) const {
		return step * delta.in_milliseconds();
	}
	
	bool stepped_timestamp::was_set() const {
		return step != static_cast<unsigned>(-1);
	}

	stepped_cooldown::stepped_cooldown(
		const real32 cooldown_duration_ms
	) : 
		cooldown_duration_ms(cooldown_duration_ms) 
	{
	}

	void stepped_cooldown::set(
		const real32 ms, 
		const stepped_timestamp now
	) {
		cooldown_duration_ms = ms;
		when_last_fired = now;
	}

	bool stepped_cooldown::lasts(
		const stepped_clock& clk
	) const {
		return clk.lasts(cooldown_duration_ms, when_last_fired);
	}

	bool stepped_cooldown::try_to_fire_and_reset(
		const stepped_clock& clk
	) {
		return clk.try_to_fire_and_reset(cooldown_duration_ms, when_last_fired);
	}

	bool stepped_cooldown::is_ready(
		const stepped_clock& clk
	) const {
		return clk.is_ready(cooldown_duration_ms, when_last_fired);
	}

	real32 stepped_cooldown::get_remaining_ms(
		const stepped_clock& clk
	) const {
		return clk.get_remaining_ms(cooldown_duration_ms, when_last_fired);
	}

	real32 stepped_cooldown::get_ratio_of_remaining_time(
		const stepped_clock& clk
	) const {
		return clk.get_ratio_of_remaining_time(cooldown_duration_ms, when_last_fired);
	}
}