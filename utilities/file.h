#pragma once

#include <vector>
#include <string>

namespace augs {
	class file_in {

	};

	extern std::vector<std::wstring> get_all_files_in_directory(std::wstring dir);
	extern std::string get_file_contents(std::string filename);
	extern std::string get_file_contents(std::wstring filename);
}