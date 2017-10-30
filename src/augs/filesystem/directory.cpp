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

	path_type get_executable_directory() {
		return fs::current_path();
	}
}
