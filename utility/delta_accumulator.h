#pragma once
#include "timer.h"

namespace augmentations {
	namespace util {
		class delta_accumulator {
			timer ticks;

			double accumulator;
			double ratio;
			double fixed_dt_milliseconds;

			double time_multiplier;

			/* maximum steps taken to avoid spiral of death */
			unsigned max_steps;
		public:
			delta_accumulator(double fps, unsigned max_steps);

			/* resets the timer and sets accumulator to 0 */
			void reset();
			/* resets only timer leaving accumulator itself unchanged */
			void reset_timer();

			unsigned update_and_extract_steps();
			double get_ratio() const;

			/* a scalar that you should multiply your numbers by to get speed per second */
			double per_second() const;
			
			/* get fixed_dt_milliseconds */
			double get_timestep() const;

			void set_time_multiplier(double);
		};
	}
}