#pragma once
#include "augs/network/netcode_sockets.h"

struct my_address_info {
	struct resolution_state {
		net_time_t when_last = 0;
		std::optional<netcode_address_t> resolved;
		netcode_address_t destination;

		bool completed() const {
			return resolved != std::nullopt;
		}
	};

	resolution_state first;
	resolution_state second;
};

struct my_network_details_gui_state : public standard_window_mixin<my_network_details_gui_state> {
	using base = standard_window_mixin<my_network_details_gui_state>;
	using base::base;

	my_address_info info;

	void advance_address_resolution(netcode_socket_t& udp_socket, const netcode_address_t& resolution_host, int);
	void handle_response(const netcode_address_t& resolution_host, const netcode_address_t& resolved_addr);
	void perform();
	void reset();
};
