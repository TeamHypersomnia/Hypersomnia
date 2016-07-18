#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"

class processing_system;

namespace components {
	struct trace {
		std::pair<float, float> max_multiplier_x = std::make_pair(1.f, 1.f);
		std::pair<float, float> max_multiplier_y = std::make_pair(1.f, 1.f);

		vec2 chosen_multiplier = vec2(-1.f, -1.f);

		std::pair<float, float> lengthening_duration_ms = std::pair<float, float>(200.f, 400.f);
		float chosen_lengthening_duration_ms = -1.f;
		float lengthening_time_passed_ms = 0.f;

		bool is_it_finishing_trace = false;

		void reset(randomization& p) {
			lengthening_time_passed_ms = 0.f;
			chosen_multiplier.set(p.randval(max_multiplier_x), p.randval(max_multiplier_y));
			chosen_lengthening_duration_ms = p.randval(lengthening_duration_ms);
		}
	};
}