#pragma once
#include <vector>
#include "application/network/address_and_port.h"

struct nat_traversal_settings {
	// GEN INTROSPECTOR struct nat_traversal_settings
	int short_ttl = 2;

	address_and_port port_probing_host;
	int num_ports_probed = 5;

	std::vector<address_and_port> stun_server_list;
	int num_stun_hosts_used_for_detection = 2;

	uint32_t request_interval_ms = 200;
	uint32_t packet_interval_ms = 10;
	// END GEN INTROSPECTOR
};

