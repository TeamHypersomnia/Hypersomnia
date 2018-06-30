#pragma once
#include "augs/misc/enum/enum_array.h"

/*

	TODO: Some render layers might correspond to distinct entity types.
	Rendering performance could be vastly improved by assigning an ordering based on these types,
	as we would avoid dispatching per-entity.
*/

enum class render_layer {
	// GEN INTROSPECTOR enum class render_layer
	INVALID,
	WANDERING_PIXELS_EFFECTS,
	NEON_CAPTIONS,
	ILLUMINATING_PARTICLES,
	ILLUMINATING_SMOKES,
	DIM_SMOKES,
	FLYING_BULLETS,
	SENTIENCES,
	DIM_WANDERING_PIXELS_EFFECTS,
	GLASS_BODY,
	SMALL_DYNAMIC_BODY,
	DYNAMIC_BODY,
	CAR_WHEEL,
	CAR_INTERIOR,
	AQUARIUM_BUBBLES,
	UPPER_FISH,
	BOTTOM_FISH,
	AQUARIUM_DUNES,
	AQUARIUM_FLOWERS,
	ON_ON_FLOOR,
	ON_FLOOR,
	FLOOR_AND_ROAD,
	GROUND,
	UNDER_GROUND,

	COUNT
	// END GEN INTROSPECTOR
};

template <class T>
using per_render_layer_t = augs::enum_array<T, render_layer>;