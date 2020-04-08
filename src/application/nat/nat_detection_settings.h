#pragma once
#include <vector>
#include "application/network/address_and_port.h"
#include "augs/filesystem/path_declaration.h"

struct nat_detection_settings {
	// GEN INTROSPECTOR struct nat_detection_settings
	address_and_port port_probing_host;
	int num_ports_probed = 5;

	augs::path_type stun_server_list = "web/stun_server_list.txt";
	int num_stun_hosts_used_for_detection = 2;

	uint32_t stun_session_timeout_ms = 1000;
	uint32_t request_interval_ms = 200;
	uint32_t packet_interval_ms = 5;
	// END GEN INTROSPECTOR
};

