#pragma once
#include <string>

struct self_update_settings {
	// GEN INTROSPECTOR struct self_update_settings
	bool update_on_launch = true;

	int update_connection_timeout_secs = 5;
	std::string update_host = "hypersomnia.io";
	std::string update_path = "/builds/latest";
	// END GEN INTROSPECTOR

	bool operator==(const self_update_settings& b) const = default;
};
