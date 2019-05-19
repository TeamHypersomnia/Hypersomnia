#pragma once
#include "augs/math/vec2.h"
#include "game/enums/startle_type.h"
#include "game/detail/render_layer_filter.h"

class cosmos;

void startle_nearby_organisms(
	cosmos& cosm,
	vec2 startle_origin,
	real32 startle_radius,
	real32 startle_force,
	startle_type,
	render_layer_filter = render_layer_filter::whitelist(
		render_layer::UPPER_FISH,
		render_layer::BOTTOM_FISH,
		render_layer::INSECTS
	)
);
