#pragma once
#include "augs/misc/enum/enum_array.h"

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
	SMALL_DYNAMIC_BODY,
	DYNAMIC_BODY,
	CAR_WHEEL,
	CAR_INTERIOR,
	ON_GROUND,
	GROUND,
	UNDER_GROUND,

	COUNT
	// END GEN INTROSPECTOR
};

template <class T>
using per_render_layer_t = augs::enum_array<T, render_layer>;