#pragma once
#include <vector>
#include <string>

#include <experimental/filesystem>

#include "augs/filesystem/path.h"

namespace augs {
	bool create_directory(const path_type& dir_path);
	bool create_directories(path_type dir_path);

	std::vector<path_type> get_all_files_in_directory(const path_type& dir);

	template <class F>
	void for_each_file_in_dir_recursive(
		const path_type& path,
		F callback
	) {
		namespace fs = std::experimental::filesystem;

		for (fs::recursive_directory_iterator i(path), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				callback(i->path().generic_string());
			}
		}
	}

	path_type get_executable_directory();
}