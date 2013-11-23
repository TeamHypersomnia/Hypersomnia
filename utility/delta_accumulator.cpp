#pragma once
#include "delta_accumulator.h"
#include <algorithm>
#include <cassert>
#undef min
/* credits goes to http://www.unagames.com/blog/daniele/2010/06/fixed-time-step-implementation-box2d */

namespace augmentations {
	namespace util {
		delta_accumulator::delta_accumulator(double fps, unsigned max_steps) : max_steps(max_steps), accumulator(0.0), fixed_dt_milliseconds((1.0/fps) * 1000.0), ratio(0.0) {
			reset_timer();
		}

		unsigned delta_accumulator::update_and_extract_steps() {
			auto add = ticks.extract<std::chrono::milliseconds>() * time_multiplier;
			//if(add > fixed_dt_milliseconds*max_steps) {
			//	add = fixed_dt_milliseconds*max_steps;
			//}
			accumulator += add;

			const unsigned steps = static_cast<unsigned>(std::floor(accumulator / fixed_dt_milliseconds));

			/* to avoid rounding errors we touch accumulator only once */
			if(steps > 0) accumulator -= steps * fixed_dt_milliseconds;
	
			assert (
				"Accumulator must have a value lesser than the fixed time step" &&
				accumulator < fixed_dt_milliseconds + FLT_EPSILON
				);

			return std::min(steps, max_steps);
		}

		double delta_accumulator::get_ratio() const {
			return accumulator / fixed_dt_milliseconds;
		}
			
		void delta_accumulator::reset_timer() {
			ticks.reset();
		}

		void delta_accumulator::reset() {
			accumulator = ratio = 0.0;
			reset_timer();
		}
		
		double delta_accumulator::per_second() const {
			/* it's 1/fps */
			return fixed_dt_milliseconds/1000.0; 
		}

		double delta_accumulator::get_timestep() const {
			return fixed_dt_milliseconds;
		}

		void delta_accumulator::set_time_multiplier(double tm) {
			time_multiplier = tm;
		}
	}
}
