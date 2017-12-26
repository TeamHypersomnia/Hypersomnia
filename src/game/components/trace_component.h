#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/minmax.h"
#include "augs/pad_bytes.h"

#include "game/transcendental/entity_type_declaration.h"

struct randomization;

namespace components {
	struct trace {
		using minmax = augs::minmax<float>;

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

		entity_type_id finishing_trace_type = 0;
		// END GEN INTROSPECTOR

		void reset(randomization& p);
	};
}