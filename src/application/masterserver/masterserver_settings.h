#pragma once
#include "augs/network/port_type.h"

struct masterserver_settings {
	// GEN INTROSPECTOR struct masterserver_settings
	std::string ip = "127.0.0.1";
	port_type port = 8414;
	unsigned server_entry_timeout_secs = 60;

	augs::path_type cert_pem_path;
	augs::path_type key_pem_path;
	// END GEN INTROSPECTOR
};
