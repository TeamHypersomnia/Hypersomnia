#pragma once
#include <memory>
#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"
#include "augs/misc/timing/fixed_delta_timer.h"
#include "view/mode_gui/arena/arena_mode_gui.h"

struct cosmos_solvable_significant;

struct editor_player {
	// GEN INTROSPECTOR struct editor_player
	double speed = 1.0;
	int additional_steps = 0;
	bool paused = true;

	all_modes_variant current_mode;
	mode_vars_id current_mode_vars_id = mode_vars_id();
	mode_entropy total_collected_entropy;
	std::unique_ptr<cosmos_solvable_significant> mode_initial_signi;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	// END GEN INTROSPECTOR

	auto get_speed() const {
		return paused ? 0.0 : speed;
	}

	/* A convenience synonym */
	bool is_editing_mode() const {
		return paused;
	}

	void control(const cosmic_entropy& entropy) {
		total_collected_entropy.cosmic += entropy;
	}

	void request_step() {
		++additional_steps;
	}
};
