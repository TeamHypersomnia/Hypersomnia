#pragma once
#include "augs/math/vec2.h"
#include "game/enums/startle_type.h"

class cosmos;

void startle_nearby_organisms(
	cosmos& cosm,
	vec2 startle_origin,
	real32 startle_radius,
	real32 startle_force,
	startle_type
);
