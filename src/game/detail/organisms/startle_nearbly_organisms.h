#pragma once
#include "augs/math/vec2.h"
#include "game/enums/startle_type.h"
#include "game/detail/render_layer_filter.h"
#include "game/detail/organisms/scare_source.h"

class cosmos;

void startle_nearby_organisms(
	cosmos& cosm,
	vec2 startle_origin,
	real32 startle_radius,
	real32 startle_force,
	startle_type,
	scare_source = scare_source::GUNS_AND_EXPLOSIONS
);
