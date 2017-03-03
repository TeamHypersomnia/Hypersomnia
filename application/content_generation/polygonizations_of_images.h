#pragma once
#include <vector>
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include <chrono>

struct polygonization_of_image_metadata {
	std::chrono::system_clock::time_point last_write_time_of_source;
};

void regenerate_polygonizations_of_images();