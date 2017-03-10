#pragma once
#include "timer.h"
#include "delta.h"

namespace augs {
	class fixed_delta_timer {
		timer ticks;

		double accumulator = 0.0;
		float time_multiplier = 1.0;

		unsigned max_steps_to_perform;
	public:
		fixed_delta_timer(unsigned max_steps_to_perform);

		void reset_timer();

		unsigned count_logic_steps_to_perform(const delta&);
		float fraction_of_step_until_next_step(const delta&) const;
		
		void set_stepping_speed_multiplier(float);
		float get_stepping_speed_multiplier() const;
	};
}