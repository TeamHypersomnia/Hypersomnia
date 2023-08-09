#pragma once
#include "augs/misc/timing/stepped_timing.h"

struct beep_math {
	const components::hand_fuse& fuse;
	const invariants::hand_fuse& fuse_def;
	const augs::stepped_clock& clk;

	auto get_beep_duration() const {
		const auto remaining_time_ms = augs::stepped_clock{ clk.dt, fuse.when_last_beep }.get_remaining_ms(
			fuse.fuse_delay_ms,
			fuse.when_armed
		);

		return std::clamp(remaining_time_ms * fuse_def.beep_time_mult, 16.f * 4, 2000.f);
	}

	auto get_beep_light_mult() const {
		const auto d = get_beep_duration();

		if (d > AUGS_EPSILON<real32>) {
			const auto remaining_ratio = clk.get_ratio_of_remaining_time(d, fuse.when_last_beep);
			return remaining_ratio;
		}

		return 1.f;
	}

	bool should_beep_again() const {
		return get_beep_light_mult() <= 0.f;
	}
};
