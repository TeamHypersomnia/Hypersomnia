#pragma once
#include "augs/math/transform.h"

#include "augs/drawing/drawing.h"

template <class T>
struct basic_drawing_input_base {
	const augs::drawer output;
	
	basic_drawing_input_base(const augs::drawer output)	: 
		output(output)
	{}

	basic_transform<T> renderable_transform;
	rgba colorize = white;
	bool use_neon_map = false;
};

using drawing_input_base = basic_drawing_input_base<real32>;