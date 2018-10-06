#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/rgba.h"
#include "augs/filesystem/path.h"
#include "augs/filesystem/file_time_type.h"

struct desaturation_stamp {
	// GEN INTROSPECTOR struct desaturation_stamp
	augs::file_time_type last_write_time_of_source;
	// END GEN INTROSPECTOR
};

void regenerate_desaturation(
	const augs::path_type& source_path,
	const augs::path_type& output_path,
	const bool force_regenerate
);