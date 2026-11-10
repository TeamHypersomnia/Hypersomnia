#pragma once
#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include "augs/string/typesafe_sprintf.h"
#include "augs/templates/container_templates.h"
#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/modes/all_mode_includes.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"
#include "application/setups/editor/playtest_recording/playtest_replay_speed_ranges.h"

struct editor_playtest_replay_snapshot {
	cosmos_solvable_significant solvable;
	all_modes_variant mode;
};

struct editor_playtest_replay {
	editor_recorded_playtest_entry recording;
	editor_playtest_recording_inputs inputs;
	synced_dynamic_vars dynamic_vars_before_replay;
	camera_eye camera_before_replay;

	uint32_t current_step = 0;
	std::optional<uint32_t> stop_at_step;
	std::optional<uint32_t> requested_seek;
	std::optional<uint32_t> first_desync_step;
	std::map<uint32_t, editor_playtest_replay_snapshot> snapshots;

	bool paused = false;
	bool controls_hidden = false;
	bool free_camera = false;
	bool reported_last_step = false;

	uint32_t get_num_steps() const {
		return recording.meta.num_steps;
	}

	uint32_t get_last_step() const {
		return std::min(get_num_steps(), stop_at_step.value_or(get_num_steps()));
	}

	bool reached_last_step() const {
		return current_step >= get_last_step();
	}

	double get_secs_per_step() const {
		if (get_num_steps() == 0) {
			return 0.0;
		}

		return recording.meta.duration_secs / get_num_steps();
	}

	double get_current_secs() const {
		return current_step * get_secs_per_step();
	}

	double get_speed_mult() const {
		return ::calc_replay_speed_mult(recording.meta.speed_ranges, get_current_secs());
	}

	uint32_t secs_to_step(const double secs) const {
		const auto secs_per_step = get_secs_per_step();

		if (secs_per_step <= 0.0 || secs <= 0.0) {
			return 0;
		}

		return std::min(get_num_steps(), static_cast<uint32_t>(secs / secs_per_step + 0.5));
	}

	void seek_to(const uint32_t step) {
		requested_seek = std::min(step, get_num_steps());
	}

	void seek_by_secs(const double secs) {
		const auto target = std::max(0.0, get_current_secs() + secs);
		seek_to(secs_to_step(target));
	}

	void resume_or_restart_at_end() {
		if (reached_last_step()) {
			seek_to(0);
		}

		paused = false;
	}

	const synced_dynamic_vars& dynamic_vars_at(const uint32_t step) const {
		const auto& vars = inputs.dynamic_vars_from_step;
		auto it = vars.upper_bound(step);

		if (it == vars.begin()) {
			return inputs.dynamic_vars_before_setup;
		}

		return std::prev(it)->second;
	}

	mode_entropy entropy_at(const uint32_t step) const {
		if (const auto recorded = mapped_or_nullptr(inputs.entropies, step); recorded != nullptr) {
			return *recorded;
		}

		return mode_entropy();
	}

	bool snapshot_missing_at_current_step() const {
		return snapshots.find(current_step) == snapshots.end();
	}

	const std::pair<const uint32_t, editor_playtest_replay_snapshot>* find_snapshot_to_restore_for(const uint32_t target) const {
		auto nearest = snapshots.upper_bound(target);

		if (nearest == snapshots.begin()) {
			return nullptr;
		}

		--nearest;

		const bool going_back = target < current_step;
		const bool snapshot_skips_steps = nearest->first > current_step;

		if (going_back || snapshot_skips_steps) {
			return std::addressof(*nearest);
		}

		return nullptr;
	}

	void verify_state_after_current_step(const uint32_t state_hash) {
		const auto& hashes = inputs.state_hashes_after_steps;

		if (!first_desync_step.has_value() && current_step < hashes.size() && hashes[current_step] != state_hash) {
			first_desync_step = current_step;
		}
	}

	std::string describe_sync() const {
		if (first_desync_step.has_value()) {
			return typesafe_sprintf("desynced at step %x", *first_desync_step);
		}

		return "in sync with the recording";
	}
};
