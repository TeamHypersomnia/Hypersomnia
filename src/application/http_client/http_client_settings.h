#pragma once
#include <string>

struct http_client_settings {
	// GEN INTROSPECTOR struct http_client_settings
	bool update_on_launch = true;

	int update_connection_timeout_secs = 5;
	std::string update_host = "hypersomnia.xyz";
	std::string update_path = "/builds/latest";
	// END GEN INTROSPECTOR

	bool operator==(const http_client_settings& b) const = default;
};
