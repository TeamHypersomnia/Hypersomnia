#include "file.h"
#include <fstream>

namespace augs {
	bool file_exists(std::string filename) {
		std::ifstream infile(filename);
		return infile.good();
	}

	std::string get_file_contents(std::string filename) {
		std::string result;
		assign_file_contents(filename, result);
		return result;
	}

	std::string get_file_contents_binary(std::string filename) {
		std::string result;
		assign_file_contents(filename, result);
		return result;
	}
}