#pragma once
#include <string>

namespace augs {
	bool spawn_detached_process(const std::string& executable, const std::string& arguments);
}