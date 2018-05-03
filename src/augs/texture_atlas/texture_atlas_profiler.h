#pragma once
#include "augs/misc/measurements.h"

struct atlas_subjects_profiler {
	// GEN INTROSPECTOR struct atlas_subjects_profiler
	augs::time_measurements loading;
	augs::time_measurements blitting;
	// END GEN INTROSPECTOR
};

struct atlas_profiler {
	// GEN INTROSPECTOR struct atlas_profiler
	atlas_subjects_profiler images;
	atlas_subjects_profiler fonts;

	augs::time_measurements packing;
	augs::time_measurements saving;
	augs::time_measurements resizing_image;
	// END GEN INTROSPECTOR
};

