#include "file.h"

#include <fstream>
#include <experimental\filesystem>

#include "augs/misc/streams.h"

namespace fs = std::experimental::filesystem;

namespace augs {
	std::chrono::system_clock::time_point last_write_time(const std::string& filename) {
		const bool exists = file_exists(filename);
		
		if (!exists) {
			LOG("File not found: %x", filename);
			ensure(exists);
		}

		return fs::last_write_time(filename);
	}

	void assign_file_contents_binary(const std::string& filename, augs::stream& target) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(target.data(), size);
		target.set_write_pos(size);
	}

	std::vector<std::string> get_file_lines(const std::string& filename) {
		std::ifstream input(filename);

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