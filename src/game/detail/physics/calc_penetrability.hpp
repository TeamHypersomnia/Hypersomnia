#pragma once
#include "augs/math/declare_math.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"

/*
	How much material the bullet traverses per unit of its penetration distance.
	0 means the surface is impenetrable.

	Must stay the single source of this rule: the missile system, the weapon laser
	and the bots' penetration estimate have to agree exactly.
*/
template <class E>
real32 calc_penetrability(const E& handle) {
	auto result = 1.f;

	if (const auto* const fixtures_def = handle.template find<invariants::fixtures>()) {
		result = fixtures_def->penetrability;
	}

	if (const auto body = handle.template find<components::rigid_body>()) {
		result *= body.get_special().penetrability;
	}

	return result;
}
