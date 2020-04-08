#pragma once
#include <functional>

using log_function = std::function<void(const std::string&)>;

inline auto make_LOG() {
	return [](const std::string& a) { LOG(a); };
}

inline auto no_LOG() {
	return [](const std::string& ) { };
}
