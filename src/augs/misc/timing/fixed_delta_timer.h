#pragma once
#include "augs/misc/timing/delta.h"

namespace augs {
	enum class lag_spike_handling_type {
		DISCARD,
		CATCH_UP
	};

	class fixed_delta_timer {
		double accumulator_secs = 0.0;
		
		unsigned max_steps_to_perform_at_once = 5;
		lag_spike_handling_type mode = lag_spike_handling_type::DISCARD;

	public:
		fixed_delta_timer() = default;
		 
		fixed_delta_timer(
			const unsigned max_steps_to_perform,
			const lag_spike_handling_type mode
		);

		void advance(const delta frame_delta);

		unsigned extract_num_of_logic_steps(const delta fixed_delta);
		real32 fraction_of_step_until_next_step(const delta fixed_delta) const;
	};
}