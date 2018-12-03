#pragma once
#include <string>
#include "augs/templates/maybe.h"

namespace augs {
	struct server_listen_input {
		// GEN INTROSPECTOR struct augs::server_listen_input
		std::string ip = "127.0.0.1";
		unsigned short port = 8412;
		int max_connections = 64;
		// END GEN INTROSPECTOR
	};
}
