#pragma once
#include <array>
#include <vector>

enum render_layer : unsigned char {
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
	ON_TILED_FLOOR,
	TILED_FLOOR,
	ON_GROUND,
	GROUND,
	UNDER_GROUND,

	COUNT
};

template <class T>
struct make_array_per_layer {
	typedef typename std::array<T, static_cast<size_t>(render_layer::COUNT)> type;
};

template<class T>
using make_array_per_layer_t = typename make_array_per_layer<T>::type;