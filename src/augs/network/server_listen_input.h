#pragma once
#include <string>
#include "augs/templates/maybe.h"
#include "augs/network/port_type.h"

namespace augs {
	struct server_listen_input {
		// GEN INTROSPECTOR struct augs::server_listen_input
		std::string ip = "127.0.0.1";
		port_type port = 0;
		int slots = 64;
		// END GEN INTROSPECTOR

		bool operator==(const server_listen_input& b) const = default;
	};

	struct dedicated_server_input {
		// GEN INTROSPECTOR struct augs::dedicated_server_input
		bool dummy = false;
		// END GEN INTROSPECTOR

		bool operator==(const dedicated_server_input& b) const = default;
	};
}
