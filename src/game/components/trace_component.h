#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/minmax.h"
#include "augs/pad_bytes.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/organization/all_entity_types.h"

struct randomization;

namespace invariants {
	struct trace;
}

namespace components {
	struct trace {
		// GEN INTROSPECTOR struct components::trace
		vec2 chosen_multiplier = vec2(-1.f, -1.f);
		float chosen_lengthening_duration_ms = -1.f;
		float lengthening_time_passed_ms = 0.f;

		vec2 last_size_mult;
		vec2 last_center_offset_mult;

		bool is_it_a_finishing_trace = false;
		bool enabled = true;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR

		void reset(
			const invariants::trace&,
			randomization& p
		);
	};
}

namespace invariants {
	struct trace {
		using minmax = augs::minmax<float>;
		using finishing_trace_flavour_type = constrained_entity_flavour_id<
			components::trace,
			components::interpolation
		>; 

		// GEN INTROSPECTOR struct invariants::trace
		minmax max_multiplier_x = minmax(1.f, 1.f);
		minmax max_multiplier_y = minmax(1.f, 1.f);

		vec2 additional_multiplier;

		minmax lengthening_duration_ms = minmax(200.f, 400.f);

		finishing_trace_flavour_type finishing_trace_flavour;

		// END GEN INTROSPECTOR
	};
}