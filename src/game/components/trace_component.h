#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/bound.h"
#include "augs/pad_bytes.h"
#include "augs/drawing/sprite.h"

#include "game/components/sprite_component_declaration.h"

namespace invariants {
	/*
		Purely cosmetic - the stretching of a flying round's sprite and the shrinking
		of what is left of it once it dies are calculated and drawn by the view alone.

		See calc_trace_scaling.h and finishing_trace_system.h.
	*/

	struct trace {
		using bound = augs::bound<float>;

		// GEN INTROSPECTOR struct invariants::trace
		bound max_multiplier_x = bound(1.f, 1.f);
		bound max_multiplier_y = bound(1.f, 1.f);

		vec2 additional_multiplier;

		bound lengthening_duration_ms = bound(200.f, 400.f);

		invariants::sprite finishing_sprite;

		bool enabled = true;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}
