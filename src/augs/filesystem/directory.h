#pragma once
#include <vector>
#include <string>

#include <experimental/filesystem>

#include "augs/filesystem/path.h"

namespace augs {
	bool create_directory(const path_type& dir_path);
	bool create_directories(path_type dir_path);

	path_type get_current_working_directory();
}