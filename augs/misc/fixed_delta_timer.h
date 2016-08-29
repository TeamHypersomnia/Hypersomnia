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
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(accumulator),
				CEREAL_NVP(time_multiplier),
				CEREAL_NVP(basic_delta),
				CEREAL_NVP(max_steps_to_perform)
			);
		}

		fixed_delta_timer(unsigned max_steps_to_perform);

		unsigned count_logic_steps_to_perform(const fixed_delta&);
		float fraction_of_step_until_next_step(const fixed_delta&) const;
		
		void set_stepping_speed_multiplier(float);
		float get_stepping_speed_multiplier() const;
	};
}