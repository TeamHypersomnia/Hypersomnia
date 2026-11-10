#pragma once
#include <algorithm>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/log.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

constexpr double playtest_replay_min_speed_v = 0.05;
constexpr double playtest_replay_max_speed_v = 4.0;

inline playtest_replay_speed_ranges load_replay_speed_ranges(const editor_playtest_recording_paths& paths) {
	if (!augs::exists(paths.speed_ranges_json)) {
		return {};
	}

	try {
		return augs::from_json_file<playtest_replay_speed_ranges>(paths.speed_ranges_json);
	}
	catch (const std::exception& err) {
		LOG("Could not read the replay speed ranges %x: %x", paths.speed_ranges_json, err.what());
		return {};
	}
}

inline double calc_replay_speed_mult(const playtest_replay_speed_ranges& speed_ranges, const double secs) {
	for (const auto& range : speed_ranges.ranges) {
		if (secs >= range.from_secs && secs < range.to_secs) {
			return std::clamp(range.speed, playtest_replay_min_speed_v, playtest_replay_max_speed_v);
		}
	}

	return 1.0;
}
