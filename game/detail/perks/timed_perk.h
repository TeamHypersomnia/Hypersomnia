#pragma once

struct timed_perk {
	float remaining_time_ms = 0.f;
	float current_maximum_time_ms = 0.f;

	void set_for_duration(const float ms) {
		remaining_time_ms = current_maximum_time_ms = ms;
	}

	bool is_enabled() const {
		return remaining_time_ms > 0.f;
	}
};