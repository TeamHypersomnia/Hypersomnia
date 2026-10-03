#pragma once
#include "augs/math/declare_math.h"
#include "game/components/movement_component.h"

/*
	With lower logic speed, characters accelerate and brake proportionally slower in real time, so they feel floaty.

	To compensate, both the movement force and the linear damping of characters are multiplied by calc_movement_snappiness_mult.
	The top speed is their ratio, so it stays the same in logic units (and so, slowed down in real time like everything else),
	but the time it takes to reach it or to stop - inversely proportional to damping - is as short in real time as at 1.0x.

	Dashes (and portal exits) keep their slowed down structure:
	the compensation fades out with the inertia they cause.

	Defined in movement_snappiness.cpp together with the tweakable MOVEMENT_SNAPPINESS_COMPENSATION,
	so that tweaking it does not recompile everything that infers damping.
*/

real32 calc_movement_snappiness_mult(
	const components::movement& movement,
	const invariants::movement& movement_def,
	real32 logic_speed
);
