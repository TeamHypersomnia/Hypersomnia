#include "directory.h"

#include <string>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace augs {
	bool create_directory(const std::string filename) {
		return fs::create_directory(filename);
	}
	
	bool create_directories(const std::string filename) {
		if (fs::path(filename).filename() == filename) {
			return true;
		}

		return fs::create_directories(fs::path(filename).remove_filename());
	}

	std::vector<std::string> get_all_files_in_directory(const std::string dirpath) {
		std::vector<std::string> result;

		for (fs::recursive_directory_iterator i(dirpath), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				result.push_back(i->path().filename().generic_string());
			}
		}

		return std::move(result);
	}

	std::string get_executable_directory() {
		return fs::current_path().string();
	}
}
