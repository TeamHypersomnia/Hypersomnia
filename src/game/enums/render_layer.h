#pragma once
#include "augs/misc/enum/enum_array.h"

enum class render_layer : unsigned char {
	INVALID,
	OVER_CROSSHAIR,
	CROSSHAIR,
	WANDERING_PIXELS_EFFECTS,
	NEON_CAPTIONS,
	ILLUMINATING_PARTICLES,
	ILLUMINATING_SMOKES,
	DIM_SMOKES,
	FLYING_BULLETS,
	SMALL_DYNAMIC_BODY,
	DYNAMIC_BODY,
	CAR_WHEEL,
	CAR_INTERIOR,
	ON_GROUND,
	GROUND,
	UNDER_GROUND,

	COUNT
};

template <class T>
struct per_render_layer {
	typedef typename augs::enum_array<T, render_layer> type;
};

template<class T>
using per_render_layer_t = typename per_render_layer<T>::type;