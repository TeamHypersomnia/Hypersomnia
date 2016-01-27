#pragma once
#include "timer.h"

namespace augs {
	class fixed_delta_timer {
		timer ticks;

		double steps_per_second;
		double accumulator;
		double ratio;
		double fixed_dt_milliseconds;

		double time_multiplier;

		/* maximum steps taken to avoid spiral of death */
		unsigned max_steps;
	public:
		fixed_delta_timer(double steps_per_second, unsigned max_steps);

		/* resets the timer and sets accumulator to 0 */
		void reset();

		unsigned count_logic_steps_to_perform();
		double fraction_of_time_until_the_next_logic_step() const;
		void set_stepping_speed_multiplier(double);

		/* a scalar that you should multiply your numbers by to get speed per second */
		double delta_seconds() const;

		/* a scalar that you should multiply your numbers by to get speed per millisecond */
		double delta_milliseconds() const;

		double get_steps_per_second() const;
	};
}