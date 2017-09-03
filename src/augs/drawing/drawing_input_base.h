#pragma once
#include "augs/math/transform.h"

#include "augs/drawing/drawing.h"
#include "augs/math/camera_cone.h"

template <class T>
struct basic_drawing_input_base {
	const augs::drawer output;
	
	basic_drawing_input_base(const augs::drawer output)	: 
		output(output)
	{}

	basic_transform<T> renderable_transform;
	basic_camera_cone<T> camera;

	rgba colorize = white;
	bool use_neon_map = false;

	void set_global_time_seconds(const double) {}
};

using drawing_input_base = basic_drawing_input_base<real32>;