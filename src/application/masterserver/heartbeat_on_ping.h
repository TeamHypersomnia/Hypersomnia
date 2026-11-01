#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "augs/network/netcode_utils.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/misc/ed25519.h"
#include "application/masterserver/server_heartbeat.h"

/*
	A server's heartbeat sent back to a ping (gameserver_heartbeat_request) - for a server the server list does not have:
	a favorite added by hand, one on the local network found by a broadcast, or any server when the list can not be reached.

	The packet: the magic, the ping's sequence, the ports of the instances launched together (lan_*: only for the local
	network's server browsers - never asked of a server the list has), the heartbeat - never the size of a ping response,
	which is told apart by its size alone (see read_ping_response).

	Then the server's public key and its signature (server.key, Ed25519) of everything before them: nobody can answer
	in a server's name, nor change what it said - its heartbeat, its instances. The ping's sequence (random) in it:
	an old answer can't be sent again.
*/

constexpr uint64_t heartbeat_on_ping_magic_v = 0x48534842454154ULL;

struct heartbeat_on_ping_header {
	uint64_t magic = heartbeat_on_ping_magic_v;
	uint64_t sequence = -1;
	uint16_t lan_first_port = 0;
	uint16_t lan_num_ports = 0;
};

constexpr std::size_t heartbeat_on_ping_signature_bytes_v = sizeof(augs::ed25519_public_key_type) + sizeof(augs::ed25519_signature_type);

struct heartbeat_on_ping {
	heartbeat_on_ping_header header;
	server_heartbeat heartbeat;
	augs::ed25519_public_key_type server_public_key = {};
};

inline std::string heartbeat_on_ping_signed_message(const std::byte* const bytes, const std::size_t count) {
	auto message = std::string("hs-ping-v1");
	message.append(reinterpret_cast<const char*>(bytes), count);
	return message;
}

/*
	Empty without a usable key - then nothing is sent.
*/

inline std::vector<std::byte> make_heartbeat_on_ping_bytes(
	const uint64_t sequence,
	const uint16_t lan_first_port,
	const uint16_t lan_num_ports,
	server_heartbeat heartbeat,
	const augs::ed25519_seed_type& server_key,
	const augs::ed25519_public_key_type& server_public_key
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

	if (bytes.size() + heartbeat_on_ping_signature_bytes_v > max_heartbeat_bytes_v) {
		heartbeat.players_resistance.clear();
		heartbeat.players_metropolis.clear();
		heartbeat.players_spectating.clear();

		write();
	}

	const auto signature = augs::ed25519_sign(server_key, ::heartbeat_on_ping_signed_message(bytes.data(), bytes.size()));

	if (!signature.has_value()) {
		return {};
	}

	auto append = [&](const auto& array) {
		const auto* const first = reinterpret_cast<const std::byte*>(array.data());
		bytes.insert(bytes.end(), first, first + array.size());
	};

	append(server_public_key);
	append(*signature);

	return bytes;
}

/*
	Only the header: which ping it answers - checked against what was asked before the signature costs anything.
*/

inline std::optional<heartbeat_on_ping_header> read_heartbeat_on_ping_header(const uint8_t* const packet_buffer, const std::size_t packet_bytes) {
	if (packet_bytes < sizeof(heartbeat_on_ping_header) + heartbeat_on_ping_signature_bytes_v) {
		return std::nullopt;
	}

	try {
		auto stream = augs::make_ptr_read_stream(packet_buffer, packet_bytes);
		const auto header = augs::read_bytes<heartbeat_on_ping_header>(stream);

		if (header.magic != heartbeat_on_ping_magic_v) {
			return std::nullopt;
		}

		return header;
	}
	catch (const augs::stream_read_error&) {
		return std::nullopt;
	}
}

/*
	Only one whose signature holds - by the key it carries; whose key that is, the one who asked checks.
*/

inline std::optional<heartbeat_on_ping> read_heartbeat_on_ping(const uint8_t* const packet_buffer, const std::size_t packet_bytes) {
	const auto header = ::read_heartbeat_on_ping_header(packet_buffer, packet_bytes);

	if (!header.has_value()) {
		return std::nullopt;
	}

	const auto signed_bytes = packet_bytes - heartbeat_on_ping_signature_bytes_v;

	try {
		auto stream = augs::make_ptr_read_stream(packet_buffer, signed_bytes);
		(void)augs::read_bytes<heartbeat_on_ping_header>(stream);

		auto heartbeat = augs::read_bytes<server_heartbeat>(stream);

		auto public_key = augs::ed25519_public_key_type();
		auto signature = augs::ed25519_signature_type();

		std::memcpy(public_key.data(), packet_buffer + signed_bytes, public_key.size());
		std::memcpy(signature.data(), packet_buffer + signed_bytes + public_key.size(), signature.size());

		const auto message = ::heartbeat_on_ping_signed_message(reinterpret_cast<const std::byte*>(packet_buffer), signed_bytes);

		if (!augs::ed25519_verify(public_key, message, signature)) {
			return std::nullopt;
		}

		return heartbeat_on_ping { *header, std::move(heartbeat), public_key };
	}
	catch (const augs::stream_read_error&) {
		return std::nullopt;
	}
}
