#pragma once
#include "game/modes/all_mode_includes.h"
#include "game/cosmos/cosmic_entropy.h"
#include "augs/misc/timing/fixed_delta_timer.h"

struct editor_player {
	// GEN INTROSPECTOR struct editor_player
	double speed = 1.0;
	int additional_steps = 0;
	bool paused = true;

	all_modes_variant current_mode;
	mode_vars_id current_mode_vars_id = mode_vars_id();
	cosmic_entropy total_collected_entropy;
	// END GEN INTROSPECTOR

	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	auto get_speed() const {
		return paused ? 0.0 : speed;
	}

	/* A convenience synonym */
	bool is_editing_mode() const {
		return paused;
	}

	template <class E>
	void control(const E& entropy) {
		total_collected_entropy += entropy;
	}

	void request_step() {
		++additional_steps;
	}
};
