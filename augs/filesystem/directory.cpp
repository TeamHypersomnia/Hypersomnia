#include "directory.h"
#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace augs {
	bool create_directory(std::string filename) {
		return fs::create_directory(filename);
	}
	
	bool create_directories(std::string filename) {
		return fs::create_directories(filename);
	}

	std::vector<std::string> get_all_files_in_directory(std::string dirpath) {
		std::vector<std::string> result;

		for (fs::recursive_directory_iterator i("."), end; i != end; ++i)
			if (!is_directory(i->path()))
				result.push_back(i->path().filename().generic_string());

		return result;
	}
}
