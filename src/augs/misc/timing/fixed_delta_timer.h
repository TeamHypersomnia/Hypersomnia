#pragma once
#include "augs/misc/timing/delta.h"

namespace augs {
	enum class lag_spike_handling_type {
		DISCARD,
		CATCH_UP
	};

	class fixed_delta_timer {
		double accumulator_secs = 0.0;
		
	public:
		unsigned max_steps_to_perform_at_once = 5;
		lag_spike_handling_type mode = lag_spike_handling_type::DISCARD;

		fixed_delta_timer() = default;
		 
		fixed_delta_timer(
			const unsigned max_steps_to_perform,
			const lag_spike_handling_type mode
		);

		void advance(const delta frame_delta);

		unsigned extract_num_of_logic_steps(const double inv_tickrate);
		double fraction_of_step_until_next_step(const double inv_tickrate) const;
	};
}