#pragma once
#include <vector>
#include "application/network/address_and_port.h"

struct nat_traversal_settings {
	// GEN INTROSPECTOR struct nat_traversal_settings
	int short_ttl = 2;
	int num_brute_force_packets = 25;

	double traversal_attempt_timeout_secs = 5;
	// END GEN INTROSPECTOR
};

