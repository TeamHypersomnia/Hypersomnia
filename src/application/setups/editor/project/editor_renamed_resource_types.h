#pragma once
#include <string>

inline const std::string& current_name_of_renamed_resource_type(const std::string& type) {
	static const std::string quick_test_spawn = "quick_test_spawn";

	if (type == "playtesting_spawn") {
		return quick_test_spawn;
	}

	return type;
}

inline bool is_removed_resource_type(const std::string& type) {
	return type == "debug_navigation_start" || type == "debug_navigation_end";
}
