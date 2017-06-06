#pragma once
#include <vector>
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include <chrono>

struct polygonization_of_image_stamp {
	// GEN INTROSPECTOR struct polygonization_of_image_stamp
	std::chrono::system_clock::time_point last_write_time_of_source;
	// END GEN INTROSPECTOR
};

void regenerate_polygonizations_of_images(
	const bool force_regenerate
);