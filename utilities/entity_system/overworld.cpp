#include "overworld.h"

namespace augs {
	overworld::overworld() : accumulator(60.0, 5) {

	}

	overworld::deterministic_timer::deterministic_timer(augs::overworld* overworld) 
		: overworld(overworld), step_recorded(0) {
		reset();
	}

	void overworld::deterministic_timer::reset() {
		step_recorded = overworld->current_step_number;
	}

	float overworld::deterministic_timer::get_milliseconds() const {
		return get_steps() * static_cast<float>(overworld->accumulator.get_timestep());
	}

	float overworld::deterministic_timer::extract_milliseconds() {
		float result = get_milliseconds();
		reset();
		return result;
	}

	unsigned overworld::deterministic_timer::get_steps() const {
		return overworld->current_step_number - step_recorded;
	}

	unsigned overworld::deterministic_timer::extract_steps() {
		unsigned result = get_steps();
		reset();
		return result;
	}

	void overworld::configure_stepping(float fps, int max_updates_per_step) {
		accumulator = augs::delta_accumulator(fps, max_updates_per_step);
	}
	
	overworld::deterministic_timer overworld::create_deterministic_timer() {
		return deterministic_timer(this);
	}

	void overworld::update_frame_timer() {
		frame_time = frame_timer.extract<std::chrono::seconds>();
	}

	double overworld::get_frame_time() {
		return frame_time;
	}
}