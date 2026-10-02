#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "augs/templates/hash_templates.h"
#include "game/components/trace_component.h"

struct trace_scaling {
	vec2 size_mult;
	vec2 center_offset_mult;
};

/*
	Decorrelates the trace's randomization from the other users
	of the round's nontemporal seed, e.g. its muzzle velocity.
*/

constexpr uint32_t trace_rng_salt = 0x74726163;

/*
	How much a flying round's sprite is stretched, and how far back its center is shifted.

	Purely cosmetic - a function of the round's age alone, so the view
	can evaluate it in between steps without keeping any state.

	steps_alpha is the same interpolation alpha the round's position is drawn with:
	at 1.0 the result equals the state at the end of the newest step,
	at 0.0 the state at the end of the step before it.
*/

template <class H>
std::optional<trace_scaling> calc_trace_scaling(const H& handle, const float steps_alpha) {
	const auto& trace_def = handle.template get<invariants::trace>();

	if (!trace_def.enabled) {
		return std::nullopt;
	}

	const auto& cosm = handle.get_cosmos();

	auto rng = randomization(augs::hash_multiple(cosm.get_nontemporal_rng_seed_for(handle.get_id()), trace_rng_salt));

	const auto chosen_multiplier = [&]() {
		const auto chosen_x = rng.randval(trace_def.max_multiplier_x);
		const auto chosen_y = rng.randval(trace_def.max_multiplier_y);

		return vec2(chosen_x, chosen_y);
	}();

	const auto chosen_lengthening_duration_ms = rng.randval(trace_def.lengthening_duration_ms);

	const auto dt_ms = static_cast<float>(cosm.get_fixed_delta().in_milliseconds());

	/*
		The step the round was born in already applies the stretch of age zero,
		so the end of the newest step shows (steps passed - 1 - birth step) steps of age.
	*/

	const auto age_in_steps =
		static_cast<double>(cosm.get_total_steps_passed())
		- static_cast<double>(handle.when_born().step)
		- 2.0
		+ static_cast<double>(steps_alpha)
	;

	const auto time_passed_ms = std::clamp(
		static_cast<float>(age_in_steps * dt_ms),
		0.f,
		chosen_lengthening_duration_ms
	);

	auto surplus_multiplier = vec2(chosen_multiplier * time_passed_ms / chosen_lengthening_duration_ms);

	/*
		Cap the lengthwise stretch so that the sprite's rear tip never reaches
		behind the point of the shot. The rear extends w * (1 + s)^2 / 2 behind
		the body (with additional_multiplier = 1), while the body has only travelled
		speed * time - and the rendered, interpolated position lags up to one tick
		behind the logical one, so one tick's worth of distance is subtracted.
		Otherwise, with short lengthening durations and low tickrates, the stretched
		trace - and the trail streams anchored at its back - poked into the shooter.
	*/
	{
		const auto w = static_cast<float>(handle.get_logical_size().x);
		const auto speed = handle.get_effective_velocity().length();

		const auto travelled = speed * std::max(0.f, time_passed_ms - dt_ms) / 1000.f;

		if (w > 0.f) {
			const auto max_surplus = std::sqrt(std::max(0.f, 2.f * travelled / w)) - 1.f;
			surplus_multiplier.x = std::min(surplus_multiplier.x, std::max(0.f, max_surplus));
		}
	}

	return trace_scaling {
		trace_def.additional_multiplier + surplus_multiplier,
		surplus_multiplier / 2.f
	};
}
