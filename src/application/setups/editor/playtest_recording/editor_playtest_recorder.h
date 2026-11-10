#pragma once
#include <cstdint>
#include "augs/math/vec2.h"
#include "augs/misc/date_time.h"
#include "augs/network/network_types.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

struct editor_playtest_start {
	vec2 initial_spawn_pos;
	uint32_t rng_offset = 0;
	client_nickname_type nickname;
};

struct editor_playtest_recorder {
	static constexpr double min_kept_secs_v = 2.0;

	augs::date_time started_at;
	editor_project project_at_start;
	editor_playtest_recording_meta meta;
	editor_playtest_recording_inputs inputs;

	void record_step(
		const mode_entropy& total,
		const synced_dynamic_vars& dynamic_vars_in_effect,
		const uint32_t state_hash_after_step,
		const double step_secs
	) {
		const auto step = meta.num_steps;

		auto& vars = inputs.dynamic_vars_from_step;

		if (vars.empty() || !(vars.rbegin()->second == dynamic_vars_in_effect)) {
			vars[step] = dynamic_vars_in_effect;
		}

		if (!total.empty()) {
			inputs.entropies[step] = total;
		}

		inputs.state_hashes_after_steps.push_back(state_hash_after_step);

		++meta.num_steps;
		meta.duration_secs += step_secs;
	}

	bool next_step_reinfers() const {
		return ::is_recorded_playtest_reinference_step(meta.num_steps);
	}

	bool too_short_to_keep() const {
		return meta.duration_secs < min_kept_secs_v;
	}
};
