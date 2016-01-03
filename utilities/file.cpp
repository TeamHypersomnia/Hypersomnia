#include "file.h"
#include "dirent.h"
#include <cassert>
#include <fstream>

namespace augs {
	bool file_exists(std::wstring filename) {
		std::ifstream infile(filename);
		return infile.good();
	}

	std::vector<std::wstring> get_all_files_in_directory(std::wstring dirpath) {
		std::vector<std::wstring> out;

		std::string dirpaths(dirpath.begin(), dirpath.end());

		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(dirpaths.c_str())) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				std::string d_name(ent->d_name);
				std::wstring wd_name(d_name.begin(), d_name.end());
				out.push_back(wd_name);
			}
			closedir(dir);
		}
		else 
			assert(0);

		return out;
	}

	std::string get_file_contents(std::string filename) {
		return get_file_contents(std::wstring(filename.begin(), filename.end()));
	}

	std::string get_file_contents(std::wstring filename) {
		std::ifstream t(filename);
		std::string script_file;

		t.seekg(0, std::ios::end);
		script_file.reserve(static_cast<unsigned>(t.tellg()));
		t.seekg(0, std::ios::beg);

		script_file.assign((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		return script_file;
	}
}