#pragma once
#include "augs/misc/profiler_mixin.h"

class frame_profiler : public augs::profiler_mixin<frame_profiler> {
public:
	frame_profiler();

	// GEN INTROSPECTOR class frame_profiler
	augs::time_measurements total;
	augs::amount_measurements<std::size_t> num_triangles;

	augs::time_measurements rendering_script;
	augs::time_measurements light_visibility;
	augs::time_measurements light_rendering;
	augs::amount_measurements<std::size_t> num_visible_lights;

	augs::time_measurements particles_rendering;
	// END GEN INTROSPECTOR
};

