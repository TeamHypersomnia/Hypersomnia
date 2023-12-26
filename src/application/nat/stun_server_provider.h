#pragma once
#include "augs/filesystem/path_declaration.h"

struct host_with_default_port;

using stun_counter_type = int;

struct stun_server_provider {
	std::vector<std::string> servers;
	std::vector<net_time_t> usage_timestamps;

	stun_counter_type current_stun_server = 0;
	stun_counter_type current_probed_port = 0;

	stun_server_provider(const augs::path_type& list_file);
	void load(const augs::path_type& list_file);
	host_with_default_port get_next();

	host_with_default_port get_next_port_probe(const nat_port_probing_settings&);

	double seconds_to_wait_for_next(double usage_cooldown_secs) const;
};
