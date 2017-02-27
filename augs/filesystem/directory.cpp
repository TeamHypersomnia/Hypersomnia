#include "directory.h"
#include <experimental/filesystem>
#include <string>

#include <windows.h>

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
		HMODULE hModule = GetModuleHandleW(NULL);
		WCHAR path[MAX_PATH];
		int chars = GetModuleFileNameW(hModule, path, MAX_PATH);

		std::string s1(path, path + chars);
		return s1.substr(0, s1.find_last_of("\\/"));
	}
}
