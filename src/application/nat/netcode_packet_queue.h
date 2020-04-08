#pragma once
#include "augs/network/netcode_sockets.h"

using netcode_queued_packet = std::pair<netcode_address_t, std::vector<std::byte>>;

struct netcode_packet_queue {
	std::vector<netcode_queued_packet> queue;

	net_time_t when_last = -1;

	void send_one(netcode_socket_t, log_function log_sink);
	void send_some(netcode_socket_t, double interval_ms, log_function log_sink);

	template <class... Args>
	void operator()(Args&&... args) {
		queue.emplace_back(std::forward<Args>(args)...);
	}
};
