#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

namespace augs {
	namespace fs = std::experimental::filesystem;

	bool create_directory(const path_type& path) {
		return fs::create_directory(path);
	}
	
	bool create_directories(path_type path) {
		if (path.filename() == path) {
			return true;
		}

		return fs::create_directories(path.remove_filename());
	}

	std::vector<path_type> get_all_files_in_directory(const path_type& dirpath) {
		std::vector<path_type> result;

		for_each_file_in_dir_recursive(
			dirpath, 
			[&result](const auto& path) {
				result.push_back(path);
			}
		);

		return result;
	}

	path_type get_executable_directory() {
		return fs::current_path();
	}
}
