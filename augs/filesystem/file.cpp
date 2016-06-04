#include "file.h"
#include <fstream>

namespace augs {
	bool file_exists(std::wstring filename) {
		std::ifstream infile(filename);
		return infile.good();
	}

	std::string get_file_contents(std::wstring filename) {
		return get_file_contents(std::string(filename.begin(), filename.end()));
	}

	std::string get_file_contents(std::string filename) {
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