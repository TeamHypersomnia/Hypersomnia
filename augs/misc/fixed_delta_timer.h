#pragma once
#include "timer.h"
#include "delta.h"

namespace augs {
	class fixed_delta_timer {
		timer ticks;

		double accumulator = 0.0;
		double time_multiplier = 1.0;

		fixed_delta basic_delta;

		unsigned max_steps_to_perform;
	public:
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(accumulator),
				CEREAL_NVP(time_multiplier),
				CEREAL_NVP(basic_delta),
				CEREAL_NVP(max_steps_to_perform)
			);
		}

		fixed_delta_timer(unsigned steps_per_second, unsigned max_steps_to_perform);

		unsigned count_logic_steps_to_perform();
		double fraction_of_step_until_next_step() const;
		void set_stepping_speed_multiplier(double);
		double get_stepping_speed_multiplier() const;

		void increment_total_steps_passed();

		fixed_delta get_fixed_delta() const;
	};
}