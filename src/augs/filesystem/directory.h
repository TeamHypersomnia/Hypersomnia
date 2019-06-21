#pragma once
#include <vector>
#include <string>

#include <filesystem>

#include "augs/filesystem/path.h"

namespace augs {
	bool create_directory(const path_type& dir_path);
	bool create_directories(const path_type& dir_path);

	bool create_directories_for(path_type path);

	path_type get_current_working_directory();

	template <class D, class F>
	void for_each_in_directory_recursive(
		const path_type& dir_path,
		D directory_callback,
		F file_callback
	) {
		using namespace std::filesystem;

		for (recursive_directory_iterator i(dir_path), end; i != end; ++i) {
			const auto p = i->path();

			if (is_directory(p)) {
				directory_callback(p);
			}
			else {
				file_callback(p);
			}
		}
	}
}