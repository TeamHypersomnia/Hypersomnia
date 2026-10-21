#pragma once
#include "augs/math/declare_math.h"

/*
	Sprinting halves the movement force (movement_system)
	and quarters the linear damping (infer_damping),
	so the top speed doubles while the acceleration halves.
*/

constexpr real32 sprint_movement_force_mult = 0.5f;
constexpr real32 sprint_linear_damping_mult = 0.25f;

/*
	Each step the body gets v = (v + dt * F / m) / (1 + dt * c) - see b2Island::Solve -
	which settles at v = F / (m * c) regardless of dt.
	Snappiness multiplies both F and c, so it cancels out.

	Logic pixels per second; the real speed on screen is this times the logic speed.
*/

inline real32 calc_top_movement_speed(
	const real32 acceleration_length,
	const real32 speed_mult,
	const real32 standard_linear_damping,
	const real32 body_mass,
	const bool sprinting
) {
	const auto force_mult = sprinting ? sprint_movement_force_mult : 1.f;
	const auto damping_mult = sprinting ? sprint_linear_damping_mult : 1.f;

	return
		(acceleration_length * speed_mult * force_mult)
		/ (body_mass * standard_linear_damping * damping_mult)
	;
}

/*
	The inverse: the body mass with which the top speed is the requested one -
	the same expression with the mass and the speed swapped.
*/

inline real32 calc_body_mass_for_top_movement_speed(
	const real32 acceleration_length,
	const real32 speed_mult,
	const real32 standard_linear_damping,
	const real32 top_speed,
	const bool sprinting
) {
	return ::calc_top_movement_speed(acceleration_length, speed_mult, standard_linear_damping, top_speed, sprinting);
}
