#pragma once
#include <string>
#include "augs/templates/maybe.h"

struct server_start_input {
	using maybe_string = augs::maybe<std::string>;

	// GEN INTROSPECTOR struct server_start_input
	maybe_string ipv4 = maybe_string::enabled("127.0.0.1");
	maybe_string ipv6 = maybe_string::enabled("::1");
	unsigned short port = 8412;
	// END GEN INTROSPECTOR
};
