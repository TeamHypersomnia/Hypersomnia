#include "file.h"
#include <fstream>

namespace augs {
	std::vector<std::string> get_file_lines(const std::string& filename) {
		std::ifstream input("filename.ext");

		std::vector<std::string> out;

		for (std::string line; std::getline(input, line); ) {
			out.emplace_back(line);
		}

		return std::move(out);
	}

	bool file_exists(std::string filename) {
		std::ifstream infile(filename);
		return infile.good();
	}

	std::string get_file_contents(std::string filename) {
		std::string result;
		assign_file_contents(filename, result);
		return result;
	}
}