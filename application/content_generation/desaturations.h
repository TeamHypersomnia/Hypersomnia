#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"

struct desaturation_metadata {
	std::chrono::system_clock::time_point last_write_time_of_source;
};

void regenerate_desaturations();