#pragma once
#include "augs/math/repro_math.h"
#include "game/cosmos/cosmos.h"
#include "view/audiovisual_state/systems/finishing_trace_system.h"

template <class F>
void finishing_trace_system::for_each_drawn(
	const cosmos& cosm,
	const float steps_alpha,
	F&& callback
) const {
	const auto steps_passed = cosm.get_total_steps_passed();
	const auto dt_secs = cosm.get_fixed_delta().in_seconds();

	for (const auto& t : traces) {
		if (steps_passed < t.steps_passed_when_spawned) {
			continue;
		}

		const auto steps_since_spawned = static_cast<double>(steps_passed - t.steps_passed_when_spawned);

		/*
			Drawn between the states at the ends of the two newest steps,
			the first of which is still the unshrunk round at the moment of its death.
		*/

		const auto size_mult = calc_size_mult(t, steps_since_spawned - 1.0 + steps_alpha, dt_secs);

		if (size_mult == std::nullopt) {
			continue;
		}

		auto where = [&]() {
			if (steps_since_spawned > 0.0) {
				return t.impact_transform;
			}

			auto result = t.previous_transform.interp_separate(t.impact_transform, steps_alpha, steps_alpha);

			/*
				For numerical stability, the same as interpolation_system.
			*/

			if (t.impact_transform.pos.compare_abs(t.previous_transform.pos, 0.1f)) {
				result.pos = t.impact_transform.pos;
			}

			if (repro::fabs(t.impact_transform.rotation - t.previous_transform.rotation) < 0.01f) {
				result.rotation = t.impact_transform.rotation;
			}

			return result;
		}();

		const auto tracified_size = vec2(t.sprite.size) * *size_mult;
		const auto center_offset_mult = (*size_mult - t.additional_multiplier) / 2.f;

		if (const auto center_offset = tracified_size * center_offset_mult;
			center_offset.is_nonzero()
		) {
			where.pos -= vec2(center_offset).rotate(where.rotation);
		}

		callback(t.sprite, where, *size_mult);
	}
}
