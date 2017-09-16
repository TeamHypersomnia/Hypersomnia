#pragma once
#include "augs/misc/delta.h"

namespace augs {
	class fixed_delta_timer {
		double accumulator_secs = 0.0;
		unsigned max_steps_to_perform;
	public:
		fixed_delta_timer(const unsigned max_steps_to_perform);

		void advance(const delta frame_delta);

		unsigned extract_num_of_logic_steps(const delta fixed_delta);
		real32 fraction_of_step_until_next_step(const delta fixed_delta) const;
	};
}