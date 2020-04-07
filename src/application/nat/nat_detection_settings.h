#pragma once
#include <vector>
#include "application/network/address_and_port.h"

struct nat_detection_settings {
	// GEN INTROSPECTOR struct nat_detection_settings
	address_and_port port_probing_host;
	int num_ports_probed = 5;

	std::vector<address_and_port> stun_server_list;
	int num_stun_hosts_used_for_detection = 2;

	uint32_t stun_session_timeout_ms = 1000;
	uint32_t request_interval_ms = 200;
	uint32_t packet_interval_ms = 10;
	// END GEN INTROSPECTOR

	template <class T>
	auto get_next_stun_host(T& current_stun_index) const {
		const auto& servers = stun_server_list;
		return servers[current_stun_index++ % servers.size()];
	}
};

