#pragma once
#include "augs/network/network_types.h"

struct host_with_default_port {
	// GEN INTROSPECTOR struct host_with_default_port
	address_string_type address = "127.0.0.1";
	port_type default_port = DEFAULT_GAME_PORT_V;
	// END GEN INTROSPECTOR

	bool operator==(const host_with_default_port& b) const {
		return address == b.address && default_port == b.default_port;
	}
};

