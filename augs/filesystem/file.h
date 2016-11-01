#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace augs {
	bool file_exists(std::string filename);
	std::string get_file_contents(std::string filename);

	std::vector<std::string> get_file_lines(const std::string& filename);

	template <class T>
	void write_file_binary(std::string filename, T& target) {
		std::ofstream out(filename, std::ios::out | std::ios::binary);
		out.write(target.data(), target.size());
	}

	template <class T>
	void assign_file_contents(std::string filename, T& target) {
		std::ifstream t(filename);

		t.seekg(0, std::ios::end);
		target.reserve(static_cast<unsigned>(t.tellg()));
		t.seekg(0, std::ios::beg);

		target.assign((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}

	template <class T>
	void assign_file_contents_binary(std::string filename, T& target) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(target.data(), size);
	}
}