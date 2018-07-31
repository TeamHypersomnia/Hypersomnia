#pragma once
#include "augs/misc/timing/stepped_timing.h"

struct beep_math {
	const components::hand_fuse& fuse;
	const invariants::hand_fuse& fuse_def;
	const augs::stepped_timestamp now;
	const augs::delta dt;

	auto get_beep_duration() const {
		const auto remaining_time_ms = augs::get_remaining_time_ms(
			fuse_def.fuse_delay_ms,
			fuse.when_armed,
			fuse.when_last_beep,
			dt
		);

		return std::clamp(remaining_time_ms * fuse_def.beep_time_mult, 16.f * 4, 2000.f);
	}

	auto get_beep_light_mult() const {
		const auto d = get_beep_duration();

		if (d > AUGS_EPSILON<real32>) {
			const auto remaining_ratio = augs::get_ratio_of_remaining_time(d, fuse.when_last_beep, now, dt);
			return remaining_ratio;
		}

		return 1.f;
	}

	bool should_beep_again() const {
		return get_beep_light_mult() <= 0.f;
	}
};
