#pragma once
#include "application/masterserver/gameserver_commands.h"

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
