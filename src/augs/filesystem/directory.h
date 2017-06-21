#pragma once
#include <vector>
#include <string>

#include <experimental/filesystem>

namespace augs {
	bool create_directory(const std::string& filename);
	bool create_directories(const std::string& filename);
	std::vector<std::string> get_all_files_in_directory(const std::string& dir);

	template <class P, class F>
	void for_each_file_in_dir_recursive(
		const P& path,
		F callback
	) {
		namespace fs = std::experimental::filesystem;

		for (fs::recursive_directory_iterator i(path), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				callback(i->path().filename().generic_string());
			}
		}
	}

	std::string get_executable_directory();
}