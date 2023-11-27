#pragma once
#include "application/gui/client/demo_player_gui.h"
#include "augs/misc/timing/fixed_delta_timer.h"

struct client_demo_player {
	int additional_steps = 0;
	std::string replay_failed_reason;
	augs::path_type source_path;
	demo_player_gui gui = std::string("Player");
	bool paused = false;
	demo_file_meta meta;
	demo_step default_step;

	std::optional<demo_step_num_type> requested_seek;
	std::vector<demo_step> demo_steps;
	demo_step_num_type current_step = 0;

	double speed = 1.0;
	double current_secs = 0.0;

	augs::fixed_delta_timer timer = { 30, augs::lag_spike_handling_type::CATCH_UP };

	bool control(const handle_input_before_game_input in);

	void pause() {
		paused = true;
	}

	void resume() {
		paused = false;
	}

	bool is_paused() const {
		return paused;
	}

	auto get_current_step() const {
		return current_step;
	}

	auto get_total_steps() const {
		return demo_steps.size();
	}

	auto get_current_secs() const {
		return current_secs;
	}

	void play_demo_from(const augs::path_type& p);

	void set_speed(const double new_speed) {
		speed = new_speed;
	}
	
	double get_speed() const {
		return is_paused() ? 0.0 : speed;
	}

	void seek_backward(const demo_step_num_type offset) {
		seek_to(current_step - std::min(current_step, offset));
	}

	void seek_forward(const demo_step_num_type offset) {
		seek_to(current_step + offset);
	}

	void seek_to(const demo_step_num_type n) {
		requested_seek = n;
	}

	template <class StepState>
	void advance_player(StepState advance_state) {
		if (current_step < demo_steps.size()) {
			current_secs += advance_state(demo_steps[current_step]);

			++current_step;
		}
		else {
			current_secs += advance_state(default_step);
		}
	}

	template <class RewindState>
	void rewind_player(RewindState rewind_state) {
		rewind_state();
		current_step = 0;
		current_secs = 0;
	}

	template <class StepState, class SeekingStepState, class RewindState>
	void advance(
		augs::delta frame_delta,
		StepState step_state, 
		SeekingStepState seeking_step_state, 
		RewindState rewind_state,
		const double inv_tickrate
	) {
		if (requested_seek.has_value()) {
			const auto target_step = *requested_seek;

			if (target_step < current_step) {
				rewind_player(rewind_state);
			}

			while (current_step < target_step) {
				advance_player(seeking_step_state);
			}

			requested_seek = std::nullopt;
			return;
		}

		auto steps = additional_steps;

		if (!is_paused()) {
			timer.advance(frame_delta *= speed);
			timer.max_steps_to_perform_at_once = 30;
			timer.mode = augs::lag_spike_handling_type::CATCH_UP;
			steps += timer.extract_num_of_logic_steps(inv_tickrate);
		}

		while (steps--) {
			advance_player(step_state);

			if (current_step == demo_steps.size()) {
				pause();
			}
		}

		additional_steps = 0;
	}
};

