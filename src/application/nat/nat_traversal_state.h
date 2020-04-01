#pragma once

struct nat_traversal_state {
	std::unordered_map<port_type, netcode_address_t> open_tunnels;

};
