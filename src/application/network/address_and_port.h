#pragma once
#include "augs/network/network_types.h"

struct address_and_port {
	// GEN INTROSPECTOR struct address_and_port
	address_string_type address = "127.0.0.1";
	port_type default_port = 8412;
	// END GEN INTROSPECTOR

	bool operator==(const address_and_port& b) const {
		return address == b.address && default_port == b.default_port;
	}
};

