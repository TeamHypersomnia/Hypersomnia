#pragma once
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#if PLATFORM_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

namespace augs {
	inline std::string exec(const std::string cmd) {
		std::array<char, 128> buffer;
		std::string result;
		std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);

		if (!pipe) {
			throw std::runtime_error("popen() failed!");
		}

		while (!feof(pipe.get())) {
			if (fgets(buffer.data(), 128, pipe.get()) != NULL) {
				result += buffer.data();
			}
		}

		const bool remove_trailing_newline_for_convenience = 
			result.size() > 0 && result.back() == '\n'
		;

		if (remove_trailing_newline_for_convenience) {
			result.pop_back();
		}

		return result;
	}
}