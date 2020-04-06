#pragma once
#include "application/masterserver/masterserver.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/log.h"

#include "augs/readwrite/to_bytes.h"

struct gameserver_command_wrapper {
	uint8_t marker = NETCODE_AUXILIARY_COMMAND_PACKET;
	gameserver_command payload;
};

inline std::optional<uint64_t> read_ping_response(const uint8_t* const packet_buffer, const std::size_t packet_bytes) {
	try {
		const auto response = augs::from_bytes<gameserver_ping_response>(packet_buffer, packet_bytes);
		return response.sequence;
	}
	catch (const augs::stream_read_error&) {
		return std::nullopt;
	}
}

template <class... T>
gameserver_command read_gameserver_command(T&&... args) {
	auto t = augs::from_bytes<gameserver_command_wrapper>(std::forward<T>(args)...);

	if (t.marker != NETCODE_AUXILIARY_COMMAND_PACKET) {
		throw augs::stream_read_error(
			"Failed to read gameserver_command_wrapper: it does not begin with NETCODE_AUXILIARY_COMMAND_PACKET."
		);
	}

	return t.payload;
}

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

inline void ping_this_server(netcode_socket_t socket, netcode_address_t target_server, const uint64_t sequence) {
	LOG("Pinging %x", ::ToString(target_server));

	auto bytes = make_gameserver_command_bytes(gameserver_ping_request { sequence });
	netcode_socket_send_packet(&socket, &target_server, bytes.data(), bytes.size());
}
