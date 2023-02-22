#include <algorithm>

#include "augs/ensure.h"
#include "augs/math/vec2.h"
#include "augs/misc/timing/fixed_delta_timer.h"

namespace augs {
	fixed_delta_timer::fixed_delta_timer(
		const unsigned max_steps_to_perform_at_once,
		const lag_spike_handling_type mode
	) : 
		max_steps_to_perform_at_once(max_steps_to_perform_at_once),
		mode(mode)
	{}

	void fixed_delta_timer::advance(const delta frame_delta) {
		accumulator_secs += frame_delta.in_seconds<double>();
	}

	unsigned fixed_delta_timer::extract_num_of_logic_steps(const double dt_secs) {
		const auto accumulator_steps = 
			static_cast<unsigned>(std::floor(accumulator_secs / dt_secs))
		;

		unsigned result = 0xdeadbeef;

		if (mode == lag_spike_handling_type::CATCH_UP) {
			const auto steps = std::min(
				max_steps_to_perform_at_once,
				accumulator_steps
			);

			if (steps > 0) {
				accumulator_secs -= steps * dt_secs;
			}

			result = steps;
		}
		else if (mode == lag_spike_handling_type::DISCARD) {
			const auto steps = 
				static_cast<unsigned>(std::floor(accumulator_secs / dt_secs))
			;

			if (steps > 0) {
				accumulator_secs -= steps * dt_secs;
			}

			ensure(
				"Accumulator must have a value lesser than the fixed time step" 
				&& accumulator_secs < dt_secs
			);

			result = std::min(steps, max_steps_to_perform_at_once);
		}
		else {
			ensure(false);
		}

		return result;
	}

	double fixed_delta_timer::next_step_progress_fraction(const double dt_secs) const {
		if (dt_secs <= AUGS_EPSILON<double>) {
			return 0.f;
		}

		return accumulator_secs / dt_secs;
	}
}
