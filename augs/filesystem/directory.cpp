#include "directory.h"
#include "dirent.h"
#include <direct.h>
#include "ensure.h"

namespace augs {
	void create_directory(std::wstring filename) {
		_wmkdir(filename.c_str());
	}

	void create_directory(std::string filename) {
		_mkdir(filename.c_str());
	}

	std::vector<std::string> get_all_files_in_directory(std::string dirpath) {
		std::vector<std::string> out;

		std::string dirpaths(dirpath.begin(), dirpath.end());

		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(dirpaths.c_str())) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				std::string d_name(ent->d_name);
				out.push_back(d_name);
			}
			closedir(dir);
		}
		else
			ensure(0);

		return out;
	}
}
