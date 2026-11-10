#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/filesystem/path.h"
#include "augs/misc/date_time.h"
#include "game/modes/mode_entropy.h"
#include "application/arena/synced_dynamic_vars.h"

constexpr uint32_t recorded_playtest_reinference_interval_steps_v = 1200;

inline bool is_recorded_playtest_reinference_step(const uint32_t step) {
	return step % recorded_playtest_reinference_interval_steps_v == 0;
}

struct editor_playtest_recording_meta {
	// GEN INTROSPECTOR struct editor_playtest_recording_meta
	std::string name;
	uint64_t recorded_at_unix = 0;
	std::string game_version;
	std::string game_mode;
	std::string nickname;
	vec2 initial_spawn_pos;
	uint32_t rng_offset = 0;
	bool built_showing_navmesh = false;
	uint32_t num_steps = 0;
	double duration_secs = 0.0;
	// END GEN INTROSPECTOR
};

struct editor_playtest_recording_inputs {
	// GEN INTROSPECTOR struct editor_playtest_recording_inputs
	synced_dynamic_vars dynamic_vars_before_setup;
	std::map<uint32_t, synced_dynamic_vars> dynamic_vars_from_step;
	std::map<uint32_t, mode_entropy> entropies;
	std::vector<uint32_t> state_hashes_after_steps;
	// END GEN INTROSPECTOR
};

struct playtest_replay_speed_range {
	// GEN INTROSPECTOR struct playtest_replay_speed_range
	double from_secs = 0.0;
	double to_secs = 0.0;
	double speed = 0.5;
	// END GEN INTROSPECTOR
};

struct playtest_replay_speed_ranges {
	// GEN INTROSPECTOR struct playtest_replay_speed_ranges
	std::vector<playtest_replay_speed_range> ranges;
	// END GEN INTROSPECTOR
};

struct editor_playtest_recording_paths {
	augs::path_type folder;
	augs::path_type map_json;
	augs::path_type meta_json;
	augs::path_type inputs_bin;
	augs::path_type speed_ranges_json;

	editor_playtest_recording_paths() = default;

	explicit editor_playtest_recording_paths(const augs::path_type& folder) :
		folder(folder),
		map_json(folder / "map.json"),
		meta_json(folder / "playtest.json"),
		inputs_bin(folder / "inputs.bin"),
		speed_ranges_json(folder / "speed_ranges.json")
	{}
};

inline augs::path_type get_recorded_playtests_folder(const augs::path_type& project_folder) {
	return project_folder / ".recorded_playtests";
}

struct editor_recorded_playtest_entry {
	editor_playtest_recording_paths paths;
	editor_playtest_recording_meta meta;

	augs::date_time get_recorded_at() const {
		return augs::date_time(static_cast<std::time_t>(meta.recorded_at_unix));
	}

	std::string get_display_name() const {
		if (!meta.name.empty()) {
			return meta.name;
		}

		return paths.folder.filename().string();
	}
};
