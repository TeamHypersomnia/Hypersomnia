#pragma once
#include <string>

struct http_client_settings {
	// GEN INTROSPECTOR struct http_client_settings
	bool update_at_startup = true;
	std::string update_url = "https://hypersomnia.xyz/builds/latest";
	// END GEN INTROSPECTOR
};
