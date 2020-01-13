#pragma once
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/pointer_to_buffer.h"

inline void send_punch_request(const netcode_socket_t& socket, netcode_address_t relay_host_address, const netcode_address_t& punched_server) {
	std::byte bytes[1 + sizeof(netcode_address_t)];

	auto buf = augs::pointer_to_buffer {bytes, sizeof(bytes)}; 
	auto out = augs::ptr_memory_stream { buf };

	augs::write_bytes(out, masterserver_udp_command::NAT_PUNCH_REQUEST);
	augs::write_bytes(out, punched_server);

	LOG("Requesting %x to punch %x!", ::ToString(relay_host_address), ::ToString(punched_server));

	auto s = socket;
	netcode_socket_send_packet(&s, &relay_host_address, out.data(), out.get_write_pos());
}

inline void send_ping_request(const netcode_socket_t& socket, netcode_address_t target_server, uint64_t sequence) {
	std::byte bytes[1 + sizeof(sequence)];

	auto buf = augs::pointer_to_buffer {bytes, sizeof(bytes)}; 
	auto out = augs::ptr_memory_stream { buf };

	augs::write_bytes(out, uint8_t(NETCODE_PING_REQUEST_PACKET));
	augs::write_bytes(out, sequence);

	auto s = socket;

	LOG("Pinging %x", ::ToString(target_server));

	netcode_socket_send_packet(&s, &target_server, out.data(), out.get_write_pos());
}
