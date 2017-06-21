#include "directory.h"

#include <string>

namespace fs = std::experimental::filesystem;

namespace augs {
	bool create_directory(const std::string& filename) {
		return fs::create_directory(filename);
	}
	
	bool create_directories(const std::string& filename) {
		if (fs::path(filename).filename() == filename) {
			return true;
		}

		return fs::create_directories(fs::path(filename).remove_filename());
	}

	std::vector<std::string> get_all_files_in_directory(const std::string& dirpath) {
		std::vector<std::string> result;

		for_each_file_in_dir_recursive(dirpath, 
			[&result](const auto& path) {
				result.push_back(path);
			}
		);

		return result;
	}

	std::string get_executable_directory() {
		return fs::current_path().string();
	}
}
