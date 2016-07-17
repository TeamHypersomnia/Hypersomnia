#include "directory.h"
#include "ensure.h"
#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace augs {
	void create_directory(std::string filename) {
		fs::create_directory(filename);
	}

	std::vector<std::string> get_all_files_in_directory(std::string dirpath) {
		std::vector<std::string> result;

		for (fs::recursive_directory_iterator i("."), end; i != end; ++i)
			if (!is_directory(i->path()))
				result.push_back(i->path().filename().generic_string());

		return result;
	}
}
