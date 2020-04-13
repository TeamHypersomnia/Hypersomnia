#pragma once
#include "augs/network/netcode_sockets.h"
#include "augs/misc/log_function.h"
#include "augs/network/netcode_queued_packet.h"

struct netcode_packet_queue {
	std::vector<netcode_queued_packet> queue;

	net_time_t when_last = -1;

	void send_one(netcode_socket_t, log_function log_sink);
	void send_some(netcode_socket_t, double interval_ms, log_function log_sink);

	template <class... Args>
	auto& operator()(Args&&... args) {
		queue.emplace_back(std::forward<Args>(args)...);
		return *this;
	}
};
