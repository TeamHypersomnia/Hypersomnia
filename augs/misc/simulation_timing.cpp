#include "simulation_timing.h"

namespace augs {
	double fixed_delta::delta_seconds() const {
		return fixed_delta_ms / 1000.0;
	}

	double fixed_delta::delta_milliseconds() const {
		return fixed_delta_ms;
	}

	double variable_delta::delta_seconds() const {
		return variable_delta_ms / 1000.0;
	}

	double variable_delta::delta_milliseconds() const {
		return variable_delta_ms;
	}

	double variable_delta::fixed_delta_milliseconds() const {
		return fixed_delta_ms;
	}

	double variable_delta::view_interpolation_ratio() const {
		return interpolation_ratio;
	}

	double variable_delta::frame_timestamp_seconds() const {
		return last_frame_timestamp_seconds;
	}

	simulation_timing::simulation_timing() : delta_timer(60.0, 5) {

	}

	void simulation_timing::configure_fixed_delta(float fps, int max_updates_per_step) {
		delta_timer = augs::fixed_delta_timer(fps, max_updates_per_step);
	}

	fixed_delta simulation_timing::get_fixed_delta() const {
		fixed_delta out;
		out.fixed_delta_ms = delta_timer.delta_milliseconds();

		return out;
	}

	variable_delta simulation_timing::extract_variable_delta() {
		auto vdt = frame_timer.extract<std::chrono::milliseconds>() * delta_timer.get_stepping_speed_multiplier();
		last_frame_timestamp_seconds += vdt / 1000.0;
		
		variable_delta out;
		out.fixed_delta_ms = delta_timer.delta_milliseconds();
		out.interpolation_ratio = delta_timer.fraction_of_time_until_the_next_logic_step();
		out.last_frame_timestamp_seconds = last_frame_timestamp_seconds;
		out.variable_delta_ms = vdt;

		return out;
	}
}