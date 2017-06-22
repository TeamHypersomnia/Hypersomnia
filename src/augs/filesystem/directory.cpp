#include "directory.h"
#include "file.h"

#include <string>

namespace fs = std::experimental::filesystem;

namespace augs {
	bool create_directory(const std::string& path) {
		return fs::create_directory(path);
	}
	
	bool create_directories(const std::string& path) {
		if (fs::path(path).filename() == path) {
			return true;
		}

		return fs::create_directories(fs::path(path).remove_filename());
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
