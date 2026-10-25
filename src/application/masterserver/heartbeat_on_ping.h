#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>
#include "augs/network/netcode_utils.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/masterserver/server_heartbeat.h"

/*
	A server's heartbeat sent back to a ping (gameserver_heartbeat_request) - for a server the server list does not have:
	a favorite added by hand, one on the local network found by a broadcast, or any server when the list can not be reached.

	The packet: the magic, the ping's sequence, the ports of the instances launched together (lan_*: only for the local
	network's server browsers - never asked of a server the list has), the heartbeat - never the size of a ping response,
	which is told apart by its size alone (see read_ping_response).
*/

constexpr uint64_t heartbeat_on_ping_magic_v = 0x48534842454154ULL;

struct heartbeat_on_ping_header {
	uint64_t magic = heartbeat_on_ping_magic_v;
	uint64_t sequence = -1;
	uint16_t lan_first_port = 0;
	uint16_t lan_num_ports = 0;
};

struct heartbeat_on_ping {
	heartbeat_on_ping_header header;
	server_heartbeat heartbeat;
};

inline std::vector<std::byte> make_heartbeat_on_ping_bytes(
	const uint64_t sequence,
	const uint16_t lan_first_port,
	const uint16_t lan_num_ports,
	server_heartbeat heartbeat
) {
	auto bytes = std::vector<std::byte>();

	auto write = [&]() {
		bytes.clear();

		auto ss = augs::ref_memory_stream(bytes);
		augs::write_bytes(ss, heartbeat_on_ping_header { heartbeat_on_ping_magic_v, sequence, lan_first_port, lan_num_ports });
		augs::write_bytes(ss, heartbeat);
	};

	write();

	/*
		Too many players with long nicknames for one packet - the counts stay, the names go.
	*/

	if (bytes.size() > max_heartbeat_bytes_v) {
		heartbeat.players_resistance.clear();
		heartbeat.players_metropolis.clear();
		heartbeat.players_spectating.clear();

		write();
	}

	return bytes;
}

inline std::optional<heartbeat_on_ping> read_heartbeat_on_ping(const uint8_t* const packet_buffer, const std::size_t packet_bytes) {
	try {
		auto stream = augs::make_ptr_read_stream(packet_buffer, packet_bytes);

		const auto header = augs::read_bytes<heartbeat_on_ping_header>(stream);

		if (header.magic != heartbeat_on_ping_magic_v) {
			return std::nullopt;
		}

		auto heartbeat = augs::read_bytes<server_heartbeat>(stream);

		return heartbeat_on_ping { header, std::move(heartbeat) };
	}
	catch (const augs::stream_read_error&) {
		return std::nullopt;
	}
}
