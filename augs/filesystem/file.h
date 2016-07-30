#pragma once
#include <string>

namespace augs {
	bool file_exists(std::string filename);
	std::string get_file_contents(std::string filename);
	
	template <class T>
	void assign_file_contents(std::string filename, T& target) {
		std::ifstream t(filename);

		t.seekg(0, std::ios::end);
		target.reserve(static_cast<unsigned>(t.tellg()));
		t.seekg(0, std::ios::beg);

		target.assign((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}
}