#pragma once

#include "augs/math/vec2.h"
#include "augs/misc/timer.h"

namespace augs {
	class smooth_value_field {
	public:
		vec2i discrete_value;
		vec2 value;

		vec2 target_value;

		double averages_per_sec = 20.0;
		double smoothing_average_factor = 0.5;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(discrete_value),

				CEREAL_NVP(value),
				CEREAL_NVP(target_value),
				CEREAL_NVP(averages_per_sec),
				CEREAL_NVP(smoothing_average_factor)
			);
		}

		void tick(double delta_seconds);
	};
}