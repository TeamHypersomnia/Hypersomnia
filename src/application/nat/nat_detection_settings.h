#pragma once
#include <cstdint>
#include <vector>
#include "application/network/host_with_default_port.h"
#include "augs/filesystem/path_declaration.h"
#include "all_paths.h"

struct nat_port_probing_settings {
	// GEN INTROSPECTOR struct nat_port_probing_settings
	host_with_default_port host;

	int num_available = 15;
	int num_probed_for_detection = 3;
	// END GEN INTROSPECTOR

	bool operator==(const nat_port_probing_settings&) const = default;
};

struct nat_detection_settings {
	// GEN INTROSPECTOR struct nat_detection_settings
	nat_port_probing_settings port_probing;

	augs::path_type stun_server_list = DETAIL_DIR / "web/stun_server_list.txt";
	int num_stun_hosts_used_for_detection = 2;

	double nat_translation_entry_timeout_secs = 30;

	uint32_t stun_session_timeout_ms = 1000;
	uint32_t request_interval_ms = 200;
	uint32_t packet_interval_ms = 5;
	// END GEN INTROSPECTOR

	bool operator==(const nat_detection_settings&) const = default;
};

