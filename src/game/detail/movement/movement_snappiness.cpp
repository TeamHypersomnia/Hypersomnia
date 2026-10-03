#include "game/detail/movement/movement_snappiness.h"
#include "game/detail/movement/movement_inertia.h"

/*
	MOVEMENT_SNAPPINESS_COMPENSATION:
	0 - no compensation, characters respond as slowly as everything else.
	1 - full compensation, characters respond as quickly as at 1.0x.
*/

constexpr real32 MOVEMENT_SNAPPINESS_COMPENSATION = 1.f;

static real32 calc_full_movement_snappiness_mult(const real32 logic_speed) {
	return 1.f + (1.f / logic_speed - 1.f) * MOVEMENT_SNAPPINESS_COMPENSATION;
}

/*
	Not compensated while inert: during constant inertia the character is meant to fly like a physical projectile,
	and the linear and portal inertias (caused by dashes and portals) blend the compensation out proportionally.
*/

real32 calc_movement_snappiness_mult(
	const components::movement& movement,
	const invariants::movement& movement_def,
	const real32 logic_speed
) {
	if (movement.const_inertia_ms > 0.f) {
		return 1.f;
	}

	const auto linear_inertia_mult = ::calc_linear_inertia_mult(movement, movement_def);
	const auto portal_inertia_mult = ::calc_portal_inertia_mult(movement);

	const auto full_mult = ::calc_full_movement_snappiness_mult(logic_speed);

	return 1.f + (full_mult - 1.f) * linear_inertia_mult * portal_inertia_mult;
}
