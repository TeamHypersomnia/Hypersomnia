#pragma once
#include "application/masterserver/masterserver_requests.h"
#include "application/network/resolve_address.h"

struct nat_puncher_client {
	netcode_address_t punched_server_addr;

	std::future<resolve_address_result> future_relay_host_addr;
	std::optional<netcode_address_t> relay_host_addr;

	void resolve_relay_host(const address_and_port& relay_host_addr) {
		future_relay_host_addr = async_resolve_address(relay_host_addr);
	}

	void advance_relay_host_resolution() {
		if (valid_and_is_ready(future_relay_host_addr)) {
			const auto result = future_relay_host_addr.get();

			LOG(result.report());

			if (result.result == resolve_result_type::OK) {
				relay_host_addr = result.addr;
			}
		}
	}

	bool is_resolving_host() const {
		return future_relay_host_addr.valid();
	}

	bool relay_host_resolved() const {
		return relay_host_addr != std::nullopt;
	}

	void punch_this_server(const netcode_socket_t& socket, const netcode_address_t& punched_server) {
		::punch_this_server(socket, *relay_host_addr, punched_server);
	}
};

