#pragma once
#include <vector>
#include "application/network/address_and_port.h"

struct nat_traversal_settings {
	// GEN INTROSPECTOR struct nat_traversal_settings
	int short_ttl = 2;

	unsigned request_interval_ms = 200;
	unsigned prediction_attempt_timeout_secs = 1;
	unsigned total_timeout_secs = 10;
	// END GEN INTROSPECTOR
};

