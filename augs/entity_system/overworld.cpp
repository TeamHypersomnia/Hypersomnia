#include "overworld.h"

namespace augs {
	overworld::overworld() : delta_timer(60.0, 5) {
		deterministic_generator.seed(0);
		drawing_time_generator.seed(0);
	}

	void overworld::configure_stepping(float fps, int max_updates_per_step) {
		delta_timer = augs::fixed_delta_timer(fps, max_updates_per_step);
	}
	
	double overworld::delta_seconds() const {
		return delta_ms / 1000.0;
	}

	double overworld::delta_milliseconds() const {
		return delta_ms;
	}

	void overworld::enable_drawing_time_random_generator() {
		current_generator = &drawing_time_generator;
	}

	void overworld::enable_deterministic_random_generator() {
		current_generator = &deterministic_generator;
	}

	std::mt19937& overworld::get_current_generator() {
		return *current_generator;
	}

	double overworld::fixed_delta_milliseconds() const {
		return delta_timer.delta_milliseconds();
	}

	double overworld::view_interpolation_ratio() const {
		return delta_timer.fraction_of_time_until_the_next_logic_step();
	}

	double overworld::frame_timestamp_seconds() const {
		return last_frame_timestamp_seconds;
	}

	void overworld::assign_frame_time_to_delta_for_drawing_time_systems() {
		delta_ms = frame_timer.extract<std::chrono::milliseconds>() * delta_timer.get_stepping_speed_multiplier();
		last_frame_timestamp_seconds += delta_ms / 1000.0;
	}

	void overworld::restore_fixed_delta() {
		delta_ms = fixed_delta_milliseconds();
	}
}