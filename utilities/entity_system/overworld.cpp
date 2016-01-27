#include "overworld.h"

namespace augs {
	overworld::overworld() : delta_timer(60.0, 5) {

	}

	overworld::deterministic_timer::deterministic_timer(augs::overworld* overworld) 
		: parent_overworld(overworld), step_recorded(0) {
		reset();
	}

	void overworld::deterministic_timer::reset() {
		step_recorded = parent_overworld->current_step_number;
	}

	float overworld::deterministic_timer::get_milliseconds() const {
		return get_steps() * static_cast<float>(parent_overworld->delta_timer.delta_milliseconds());
	}

	float overworld::deterministic_timer::extract_milliseconds() {
		float result = get_milliseconds();
		reset();
		return result;
	}

	unsigned overworld::deterministic_timer::get_steps() const {
		return parent_overworld->current_step_number - step_recorded;
	}

	unsigned overworld::deterministic_timer::extract_steps() {
		unsigned result = get_steps();
		reset();
		return result;
	}

	void overworld::configure_stepping(float fps, int max_updates_per_step) {
		delta_timer = augs::fixed_delta_timer(fps, max_updates_per_step);
	}
	
	overworld::deterministic_timer overworld::create_deterministic_timer() {
		return deterministic_timer(this);
	}

	double overworld::delta_seconds() {
		return delta_ms / 1000.0;
	}

	double overworld::delta_milliseconds() {
		return delta_ms;
	}

	double overworld::view_interpolation_ratio() {
		return delta_timer.fraction_of_time_until_the_next_logic_step();
	}

	void overworld::assign_frame_time_to_delta_for_drawing_time_systems() {
		delta_ms = frame_timer.extract<std::chrono::milliseconds>();
	}

	void overworld::restore_fixed_delta() {
		delta_ms = delta_timer.delta_milliseconds();
	}
}