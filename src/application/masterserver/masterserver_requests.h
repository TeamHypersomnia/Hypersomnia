#pragma once
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/log.h"

#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/gameserver_commands.h"
#include "augs/network/netcode_queued_packet.h"
#include "application/network/resolve_address.h"

template <class T>
auto make_gameserver_command_bytes(T&& command) {
	auto wrapper = gameserver_command_wrapper();
	wrapper.payload = std::forward<T>(command);

	return augs::to_bytes(wrapper);
}

template <class R>
void netcode_send_to_masterserver(netcode_socket_t socket, netcode_address_t masterserver_address, const R& typed_request) {
	auto bytes = augs::to_bytes(masterserver_request(typed_request));
	netcode_socket_send_packet(&socket, &masterserver_address, bytes.data(), bytes.size());
}

inline netcode_queued_packet make_ping_packet(netcode_address_t target_server, const uint64_t sequence) {
	auto bytes = make_gameserver_command_bytes(gameserver_ping_request { sequence });
	return { target_server, bytes };
}

inline void ping_this_server(netcode_socket_t socket, netcode_address_t target_server, const uint64_t sequence) {
	LOG("Pinging %x", ::ToString(target_server));

	auto packet = make_ping_packet(target_server, sequence);
	netcode_socket_send_packet(&socket, &packet.to, packet.bytes.data(), packet.bytes.size());
}
