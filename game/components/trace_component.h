#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "augs/misc/minmax.h"

class processing_system;

namespace components {
	struct trace {
		augs::minmax<float> max_multiplier_x = std::make_pair(1.f, 1.f);
		augs::minmax<float> max_multiplier_y = std::make_pair(1.f, 1.f);

		vec2 chosen_multiplier = vec2(-1.f, -1.f);

		augs::minmax<float> lengthening_duration_ms = std::pair<float, float>(200.f, 400.f);
		float chosen_lengthening_duration_ms = -1.f;
		float lengthening_time_passed_ms = 0.f;

		bool is_it_finishing_trace = false;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(max_multiplier_x),
				CEREAL_NVP(max_multiplier_y),

				CEREAL_NVP(chosen_multiplier),

				CEREAL_NVP(lengthening_duration_ms),
				CEREAL_NVP(chosen_lengthening_duration_ms),
				CEREAL_NVP(lengthening_time_passed_ms),

				CEREAL_NVP(is_it_finishing_trace)
			);
		}

		void reset(randomization& p) {
			lengthening_time_passed_ms = 0.f;
			chosen_multiplier.set(p.randval(max_multiplier_x), p.randval(max_multiplier_y));
			chosen_lengthening_duration_ms = p.randval(lengthening_duration_ms);
		}
	};
}