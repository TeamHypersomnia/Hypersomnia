#pragma once
#include "augs/math/vec2.h"

class cosmos;

void startle_nearby_organisms(
	cosmos& cosm,
	vec2 startle_origin,
	real32 startle_radius
);
