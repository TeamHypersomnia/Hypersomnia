#pragma once
#include "game/cosmos/logic_step.h"
#include "augs/misc/enum/enum_boolset.h"
#include "game/detail/view_input/predictability_info.h"

enum class dash_flag {
	PROPORTIONAL_TO_CURRENT_SPEED,
	COUNT
};

using dash_flags = augs::enum_boolset<dash_flag>;

template <class T>
auto perform_dash(
	const T& subject,
	const vec2 dash_dir,
	const real32 dash_impulse,
	const real32 make_inert_for_ms,
	const dash_flags flags
) {
	{
		auto& movement = subject.template get<components::movement>();
		movement.linear_inertia_ms += make_inert_for_ms;
	}

	const auto body = subject.template find<components::rigid_body>();

	if (body == nullptr) {
		return 0.f;
	}

	const auto current_velocity = body.get_velocity();
	const auto current_speed = current_velocity.length();

	const auto vel_dir = vec2(current_velocity).normalize_hint(current_speed);

	const auto impulse_mult = (vel_dir.dot(dash_dir) + 1) / 2;

	const auto& movement_def = subject.template get<invariants::movement>();
	const auto conceptual_max_speed = std::max(1.f, movement_def.max_speed_for_animation);
	const auto speed_mult = current_speed / conceptual_max_speed;

	const auto& sentience_def = subject.template get<invariants::sentience>();

	auto resultant_scale = 
		sentience_def.dash_impulse_mult 
		* impulse_mult 
		* dash_impulse
	;

	if (flags.test(dash_flag::PROPORTIONAL_TO_CURRENT_SPEED)) {
		if (current_speed < 10.f) {
			return 0.f;
		}

		resultant_scale *= std::min(1.f, speed_mult);
	}

	body.apply_impulse(dash_dir * resultant_scale);

	const auto effect_mult = speed_mult * impulse_mult;

	return effect_mult;
}

template <class T, class... Args>
auto perform_dash_towards_crosshair(
	const T& subject,
	Args&&... args
) {
	if (const auto crosshair = subject.find_crosshair()) {
		const auto cross_dir = vec2(crosshair->base_offset).normalize();
		const auto& dash_dir = cross_dir;

		return ::perform_dash(
			subject,
			dash_dir,
			std::forward<Args>(args)...
		);
	}

	return 0.f;
}

template <class T>
auto perform_dash_effects(
	const logic_step step,
	const T& subject,
	const real32 effect_mult,
	const predictability_info predictability
) {
	if (const auto movement_def = subject.template find<invariants::movement>()) {
		const auto bound = movement_def->dash_effect_bound;
		const auto chosen_mult = std::clamp(effect_mult, bound.first, bound.second);

		{
			auto effect = movement_def->dash_particles;
			effect.modifier *= chosen_mult;

			effect.start(
				step,
				particle_effect_start_input::at_entity(subject).face_velocity(),
				predictability
			);
		}

		{
			auto effect = movement_def->dash_sound;
			effect.modifier.pitch *= chosen_mult;

			effect.start(
				step,
				sound_effect_start_input::at_entity(subject).set_listener(subject).face_velocity(),
				predictability
			);
		}
	}
}
