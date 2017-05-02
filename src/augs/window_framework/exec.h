#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

namespace augs {
	std::string exec(const std::string cmd) {
		std::array<char, 128> buffer;
		std::string result;
		std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);

		if (!pipe) {
			throw std::runtime_error("popen() failed!");
		}

		while (!feof(pipe.get())) {
			if (fgets(buffer.data(), 128, pipe.get()) != NULL) {
				result += buffer.data();
			}
		}

		return result;
	}
}