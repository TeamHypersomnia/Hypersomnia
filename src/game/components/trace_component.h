#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "augs/misc/minmax.h"
#include "augs/pad_bytes.h"

class processing_system;

namespace components {
	struct trace {
		typedef augs::minmax<float> minmax;

		// GEN INTROSPECTOR struct components::trace
		minmax max_multiplier_x = minmax(1.f, 1.f);
		minmax max_multiplier_y = minmax(1.f, 1.f);

		vec2 additional_multiplier;
		vec2 chosen_multiplier = vec2(-1.f, -1.f);

		minmax lengthening_duration_ms = minmax(200.f, 400.f);
		float chosen_lengthening_duration_ms = -1.f;
		float lengthening_time_passed_ms = 0.f;

		bool is_it_a_finishing_trace = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR

		void reset(randomization& p) {
			lengthening_time_passed_ms = 0.f;
			chosen_multiplier.set(p.randval(max_multiplier_x), p.randval(max_multiplier_y));
			chosen_lengthening_duration_ms = p.randval(lengthening_duration_ms);
		}
	};
}