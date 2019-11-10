#include "augs/misc/randomization.h"

#include "game/components/trace_component.h"

namespace components {
	void trace::reset(
		const invariants::trace& def,
		randomization& p
	) {
		lengthening_time_passed_ms = 0.f;

		const auto chosen_x = p.randval(def.max_multiplier_x);
		const auto chosen_y = p.randval(def.max_multiplier_y);

		chosen_multiplier.set(chosen_x, chosen_y);
		chosen_lengthening_duration_ms = p.randval(def.lengthening_duration_ms);
	}
}