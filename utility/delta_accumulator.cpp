#pragma once
#include "delta_accumulator.h"
#include <algorithm>
#include <cassert>
#undef min
/* credits goes to http://www.unagames.com/blog/daniele/2010/06/fixed-time-step-implementation-box2d */

namespace augmentations {
	namespace util {
		delta_accumulator::delta_accumulator(double fps, unsigned max_steps) : max_steps(max_steps), accumulator(0.0), fixed_dt_miliseconds(1000.0/fps), ratio(0.0) {
			reset_timer();
		}

		unsigned delta_accumulator::update_and_extract_steps() {
			accumulator += ticks.miliseconds();

			const unsigned steps = static_cast<unsigned>(std::floor(accumulator / fixed_dt_miliseconds));

			/* to avoid rounding errors we touch accumulator only once */
			if(steps > 0) accumulator -= steps * fixed_dt_miliseconds;

			assert (
				"Accumulator must have a value lesser than the fixed time step" &&
				accumulator < fixed_dt_miliseconds + FLT_EPSILON
				);

			return std::min(steps, max_steps);
		}

		double delta_accumulator::get_ratio() const {
			return accumulator / fixed_dt_miliseconds;
		}
			
		void delta_accumulator::reset_timer() {
			ticks.microseconds();
		}

		void delta_accumulator::reset() {
			accumulator = ratio = 0.0;
			reset_timer();
		}
		
		double delta_accumulator::per_second() const {
			/* it's 1.0/fps */
			return fixed_dt_miliseconds/1000.0; 
		}
	}
}
