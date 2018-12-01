#pragma once
#include <string>
#include "augs/templates/maybe.h"

namespace augs {
	struct server_listen_input {
		using maybe_string = maybe<std::string>;

		// GEN INTROSPECTOR struct augs::server_listen_input
		maybe_string ipv4 = maybe_string::enabled("127.0.0.1");
		maybe_string ipv6 = maybe_string::enabled("::1");
		unsigned short port = 8412;
		// END GEN INTROSPECTOR
	};
}
