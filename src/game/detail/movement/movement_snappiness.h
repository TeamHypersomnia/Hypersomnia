#pragma once
#include <algorithm>
#include "augs/math/declare_math.h"
#include "game/components/movement_component.h"

/*
	With lower logic speed, characters accelerate and brake proportionally slower in real time, so they feel floaty.

	To compensate, both the movement force and the linear damping of characters are multiplied by calc_movement_snappiness_mult.
	The top speed is their ratio, so it stays the same in logic units (and so, slowed down in real time like everything else),
	but the time it takes to reach it or to stop - inversely proportional to damping - is as short in real time as at 1.0x.

	MOVEMENT_SNAPPINESS_COMPENSATION:
	0 - no compensation, characters respond as slowly as everything else.
	1 - full compensation, characters respond as quickly as at 1.0x.

	Dashes (and portal exits) keep their slowed down structure:
	the compensation fades out with the inertia they cause, see calc_movement_snappiness_mult.
*/

constexpr real32 MOVEMENT_SNAPPINESS_COMPENSATION = 1.f;

inline real32 calc_full_movement_snappiness_mult(const real32 logic_speed) {
	return 1.f + (1.f / logic_speed - 1.f) * MOVEMENT_SNAPPINESS_COMPENSATION;
}

/*
	Not compensated while inert: during constant inertia the character is meant to fly like a physical projectile,
	and the linear and portal inertias (caused by dashes and portals) blend the compensation out proportionally.
*/

inline real32 calc_movement_snappiness_mult(
	const components::movement& movement,
	const invariants::movement& movement_def,
	const real32 logic_speed
) {
	if (movement.const_inertia_ms > 0.f) {
		return 1.f;
	}

	const auto max_portal_inertia_ms = 500.f;

	const auto linear_inertia_mult = std::clamp(1.f - movement.linear_inertia_ms / movement_def.max_linear_inertia_when_movement_possible, 0.f, 1.f);
	const auto portal_inertia_mult = std::clamp(1.f - movement.portal_inertia_ms / max_portal_inertia_ms, 0.f, 1.f);

	const auto full_mult = ::calc_full_movement_snappiness_mult(logic_speed);

	return 1.f + (full_mult - 1.f) * linear_inertia_mult * portal_inertia_mult;
}
