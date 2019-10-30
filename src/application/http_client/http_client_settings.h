#pragma once
#include <string>

struct http_client_settings {
	// GEN INTROSPECTOR struct http_client_settings
	bool update_on_launch = true;

	int update_connection_timeout_secs = 5;
	std::string application_update_host = "hypersomnia.xyz";
	std::string application_update_path = "/builds/latest";
	// END GEN INTROSPECTOR
};
