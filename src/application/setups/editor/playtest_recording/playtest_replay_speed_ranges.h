#pragma once
#include <algorithm>
#include <vector>
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

constexpr double playtest_replay_min_speed_v = 0.05;
constexpr double playtest_replay_max_speed_v = 4.0;

inline double calc_replay_speed_mult(const std::vector<playtest_replay_speed_range>& speed_ranges, const double secs) {
	for (const auto& range : speed_ranges) {
		if (secs >= range.from_secs && secs < range.to_secs) {
			return std::clamp(range.speed, playtest_replay_min_speed_v, playtest_replay_max_speed_v);
		}
	}

	return 1.0;
}
