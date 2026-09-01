#pragma once
#include "augs/misc/enum/enum_array.h"

enum class particle_layer {
	// GEN INTROSPECTOR enum class particle_layer
	ILLUMINATING_PARTICLES,
	ILLUMINATING_SMOKES,
	DIM_SMOKES,
	NEONING_PARTICLES,

	/*
		Drawn in full color (standard shader, no neon) above all other particle layers.
		Gives effects like bullet line trails their own buffer, so they are never
		dropped when the other layers' particle pools overflow.
	*/
	TRAILS,

	COUNT
	// END GEN INTROSPECTOR
};

template <class T>
using per_particle_layer_t  = augs::enum_array<T, particle_layer>;
