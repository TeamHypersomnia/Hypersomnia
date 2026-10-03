#pragma once
#include <algorithm>
#include "augs/math/declare_math.h"
#include "game/components/movement_component.h"

/*
	How much of the movement (force, damping, snappiness) remains while a dash or a portal exit carries the character.
	1 - no inertia, 0 - fully inert.
*/

constexpr real32 MAX_CONSIDERED_PORTAL_INERTIA_MS = 500.f;

inline real32 calc_linear_inertia_mult(const components::movement& movement, const invariants::movement& movement_def) {
	return std::clamp(1.f - movement.linear_inertia_ms / movement_def.max_linear_inertia_when_movement_possible, 0.f, 1.f);
}

inline real32 calc_portal_inertia_mult(const components::movement& movement) {
	return std::clamp(1.f - movement.portal_inertia_ms / MAX_CONSIDERED_PORTAL_INERTIA_MS, 0.f, 1.f);
}
