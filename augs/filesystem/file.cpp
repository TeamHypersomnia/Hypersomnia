#include "file.h"

#include <fstream>
#include <experimental\filesystem>

#include "augs/misc/streams.h"

namespace fs = std::experimental::filesystem;

namespace augs {
	std::chrono::system_clock::time_point last_write_time(const std::string& path) {
		ensure_existence(path);
		return fs::last_write_time(path);
	}

	void assign_file_contents_binary(const std::string& path, augs::stream& target) {
		ensure_existence(path);
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(target.data(), size);
		target.set_write_pos(static_cast<size_t>(size));
	}

	std::vector<std::string> get_file_lines(const std::string& path) {
		ensure_existence(path);
		std::ifstream input(path);

		std::vector<std::string> out;

		for (std::string line; std::getline(input, line); ) {
			out.emplace_back(line);
		}

		return std::move(out);
	}

	void ensure_existence(const std::string& path) {
		const bool exists = file_exists(path);

		if (!exists) {
			LOG("File not found: %x", path);
			ensure(exists);
		}
	}

	bool file_exists(const std::string& path) {
		std::ifstream infile(path);
		return infile.good();
	}

	std::string get_file_contents(const std::string& path) {
		std::string result;
		assign_file_contents(path, result);
		return result;
	}
}