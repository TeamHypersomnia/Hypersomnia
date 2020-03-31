#pragma once
#include <string>
#include "augs/network/netcode_sockets.h"

struct netcode_address_t;

std::string ToString(const netcode_address_t&);
bool operator==(const netcode_address_t& a, const netcode_address_t& b);
bool operator!=(const netcode_address_t& a, const netcode_address_t& b);

bool try_fire_interval(double interval, net_time_t& when_last);
bool try_fire_interval(double interval, net_time_t& when_last, double current_time);

template <class F>
void receive_netcode_packets(netcode_socket_t socket, F&& callback) {
	netcode_address_t from;
	uint8_t packet_buffer[NETCODE_MAX_PACKET_BYTES];

	while (true) {
		const auto packet_bytes = netcode_socket_receive_packet(&socket, &from, packet_buffer, NETCODE_MAX_PACKET_BYTES);

		if (packet_bytes < 1) {
			break;
		}

		callback(from, packet_buffer, packet_bytes);
	}
}
