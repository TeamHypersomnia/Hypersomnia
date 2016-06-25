#pragma once
#include <string>

namespace augs {
	bool file_exists(std::string filename);
	bool file_exists(std::wstring filename);
	std::string get_file_contents(std::string filename);
	std::string get_file_contents(std::wstring filename);
}