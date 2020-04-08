#pragma once
#include "augs/filesystem/path_declaration.h"

struct address_and_port;

using stun_counter_type = int;

struct stun_server_provider {
	std::vector<std::string> servers;
	stun_counter_type current_stun_server = 0;

	stun_server_provider(const augs::path_type& list_file);
	void load(const augs::path_type& list_file);
	address_and_port get_next();
};
