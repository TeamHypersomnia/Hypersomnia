#pragma once

struct nat_traversal_settings {
	// GEN INTROSPECTOR struct nat_traversal_settings
	int short_ttl = 2;

	address_and_port port_probing_host;
	int num_ports_probed = 5;

	std::vector<address_and_port> stun_server_list;
	// END GEN INTROSPECTOR
};

