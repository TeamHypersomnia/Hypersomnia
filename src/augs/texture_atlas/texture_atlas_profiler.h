#pragma once
#include "augs/misc/measurements.h"

struct atlas_subjects_profiler {
	// GEN INTROSPECTOR struct atlas_subjects_profiler
	augs::time_measurements loading = std::size_t(1);
	augs::time_measurements blitting = std::size_t(1);
	// END GEN INTROSPECTOR
};

struct atlas_profiler {
	// GEN INTROSPECTOR struct atlas_profiler
	atlas_subjects_profiler images;
	atlas_subjects_profiler fonts;

	augs::amount_measurements<std::size_t> images_count = std::size_t(1);
	augs::time_measurements packing = std::size_t(1);
	augs::time_measurements saving = std::size_t(1);
	augs::time_measurements resizing_image = std::size_t(1);
	// END GEN INTROSPECTOR
};

