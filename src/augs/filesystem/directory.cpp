#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

namespace augs {
	namespace fs = std::experimental::filesystem;

	bool create_directory(const path_type& input_path) {
		return fs::create_directory(input_path);
	}
	
	bool create_directories(const path_type& input_path) {
		return fs::create_directories(input_path);
	}

	bool create_directories_for(path_type input_path) {
		if (input_path.filename() == input_path) {
			return true;
		}

		return augs::create_directories(input_path.remove_filename());
	}

	path_type get_current_working_directory() {
		return fs::current_path();
	}
}
