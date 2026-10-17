#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "augs/network/network_types.h"
#include "application/masterserver/server_heartbeat.h"

enum class server_type : uint8_t {
	NATIVE,
	WEB
};

/*
	server_public_key: the Ed25519 key the server signs its heartbeats with and seals its handshakes to; all zero if unknown.
*/

struct masterserver_entry_meta {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct masterserver_entry_meta
	double time_hosted = 0.0;
	bool is_official = false;
	host_string_type domain;
	webrtc_id_type webrtc_id;
	server_type type = server_type::NATIVE;
	std::array<uint8_t, 32> server_public_key = {};
	// END GEN INTROSPECTOR

	bool is_community_server() const {
		return !is_official;
	}

	bool is_official_server() const {
		return is_official;
	}
};

struct masterserver_client {
	double time_last_heartbeat = -1;

	/*
		The id of the official server that has signed the last heartbeat, empty for community servers.
	*/

	std::string official_server_id;

	masterserver_entry_meta meta;
	server_heartbeat last_heartbeat;
};


