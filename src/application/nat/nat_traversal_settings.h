#pragma once
#include <vector>
#include "application/network/host_with_default_port.h"
#include "augs/templates/maybe.h"

struct nat_traversal_settings {
	// GEN INTROSPECTOR struct nat_traversal_settings
	augs::maybe<int> short_ttl = augs::maybe<int>::disabled(3);
	int num_brute_force_packets = 25;

	double traversal_attempt_timeout_secs = 5;
	// END GEN INTROSPECTOR

	bool operator==(const nat_traversal_settings& b) const = default;
};

