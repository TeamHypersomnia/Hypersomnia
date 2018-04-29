#include "augs/misc/randomization.h"

#include "game/components/trace_component.h"

namespace components {
	void trace::reset(
		const invariants::trace& def,
		randomization& p
	) {
		lengthening_time_passed_ms = 0.f;
		chosen_multiplier.set(p.randval(def.max_multiplier_x), p.randval(def.max_multiplier_y));
		chosen_lengthening_duration_ms = p.randval(def.lengthening_duration_ms);
	}
}