#pragma once
#include <vector>
#include <string>

namespace augs {
	bool create_directory(std::string filename);
	bool create_directories(std::string filename);
	std::vector<std::string> get_all_files_in_directory(std::string dir);
}