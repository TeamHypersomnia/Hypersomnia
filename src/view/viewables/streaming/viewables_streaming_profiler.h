#pragma once
#include <cstddef>
#include "augs/misc/measurements.h"
#include "augs/misc/profiler_mixin.h"

struct viewables_streaming_profiler : public augs::profiler_mixin<viewables_streaming_profiler> {
	viewables_streaming_profiler();

	// GEN INTROSPECTOR struct viewables_streaming_profiler
	augs::time_measurements detecting_changed_viewables = std::size_t(1);
	augs::time_measurements launching_atlas_reload = std::size_t(1);
	augs::time_measurements launching_sounds_reload = std::size_t(1);
	// END GEN INTROSPECTOR
};
