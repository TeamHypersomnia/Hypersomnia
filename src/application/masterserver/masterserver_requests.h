#pragma once
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/log.h"

#include "application/masterserver/netcode_ping_request.h"
#include "augs/readwrite/to_bytes.h"

template <class R>
void netcode_send_to_masterserver(netcode_socket_t socket, netcode_address_t masterserver_address, const R& typed_request) {
	auto bytes = augs::to_bytes(masterserver_request(typed_request));
	netcode_socket_send_packet(&socket, &masterserver_address, bytes.data(), bytes.size());
}

inline void punch_this_server(const netcode_socket_t& socket, const netcode_address_t& masterserver_address, const netcode_address_t& punched_server) {
	LOG("Requesting %x to punch %x!", ::ToString(masterserver_address), ::ToString(punched_server));

	const auto request = masterserver_in::punch_this_server { punched_server };
	netcode_send_to_masterserver(socket, masterserver_address, request);
}

inline void tell_me_my_address(const netcode_socket_t& socket, netcode_address_t masterserver_address) {
	LOG("Requesting address resolution from %x", ::ToString(masterserver_address));

	const auto request = masterserver_in::tell_me_my_address {};
	netcode_send_to_masterserver(socket, masterserver_address, request);
}

inline void ping_this_server(netcode_socket_t socket, netcode_address_t target_server, uint64_t sequence) {
	LOG("Pinging %x", ::ToString(target_server));

	auto bytes = make_ping_request_message_bytes(sequence);
	netcode_socket_send_packet(&socket, &target_server, bytes.data(), bytes.size());
}
