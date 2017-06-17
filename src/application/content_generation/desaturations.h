#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/rgba.h"
#include "augs/misc/templated_readwrite.h"

struct desaturation_stamp {
	// GEN INTROSPECTOR struct desaturation_stamp
	std::chrono::system_clock::time_point last_write_time_of_source;
	// END GEN INTROSPECTOR
};

void regenerate_desaturation(
	const std::string& path,
	const bool force_regenerate
);

void regenerate_desaturations(
	const bool force_regenerate
);